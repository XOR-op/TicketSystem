#include "PendingTicketManager.h"
using namespace t_sys;

static void loadpendingorder(std::fstream& ifs,DiskLoc_T offset,pending_order* po){
    ifs.seekg(offset);
    ifs.read((char*)po, sizeof(pending_order));
}
static void savependingorder(std::fstream& ofs,DiskLoc_T offset,const pending_order* po){
    ofs.seekp(offset);
    ofs.write((char*)po, sizeof(pending_order));
}
PendingTicketManager::PendingTicketManager(const std::string& path) :
        pending_cache(71, [this](DiskLoc_T off, pending_order* po) { loadpendingorder(pendingFile, off, po); },
                      [this](DiskLoc_T off, const pending_order* po) { savependingorder(pendingFile, off, po); }){
    pendingFile.open(path);
    char buf2[sizeof(pending_file_size)];
    char* ptr2 = buf2;
    pendingFile.seekg(0);
    pendingFile.read(buf2, sizeof(buf2));
#define read_attribute(ATTR) memcpy((void*)&ATTR,ptr2,sizeof(ATTR));ptr2+=sizeof(ATTR)
    read_attribute(pending_file_size);
#undef read_attribute

}
PendingTicketManager::~PendingTicketManager() {
    char buf[sizeof(pending_file_size)];
    char* ptr = buf;
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
    write_attribute(pending_file_size);
#undef write_attribute
    pendingFile.seekp(0);
    pendingFile.write(buf, sizeof(buf));
    pending_cache.destruct();
    pendingFile.close();
}
void PendingTicketManager::allocate_tickets(UserOrderManager* ord_manager, train* ptr, const order* Order) {
    for (DiskLoc_T la = -1, where = ptr->ticket_head; where != -1;) {
//        assert(where);
        auto* p = pending_cache.get(where);
        bool flag = false;
        if (p->day == Order->day) {
            flag = true;
            for (int j = p->s; j < p->t; j++)
                if (p->num > ptr->stationTicketRemains[p->day][j]) {
                    flag = false;
                    break;
                }
        }
        if (flag) {
            for (int j = p->s; j < p->t; j++)
                ptr->stationTicketRemains[p->day][j]-=p->num;
            ord_manager->setSuccess(p->block, p->offset_in_block);
            //delete p
            if (where == ptr->ticket_head)ptr->ticket_head = p->nxt;
            if (where == ptr->ticket_end)ptr->ticket_end = la;
            auto* la_p = pending_cache.get(la);
            la_p->nxt = p->nxt;
            pending_cache.set_dirty_bit(la);
            pending_cache.remove(where);
            where = la_p->nxt;
        } else {
            la = where, where = p->nxt;
        }
    }
}
void PendingTicketManager::add_pendingorder(pending_order* record, train* tra) {
    DiskLoc_T rt = pending_file_size;
    if (tra->ticket_head == -1) {
        tra->ticket_head = tra->ticket_end = rt;
        pendingFile.seekp(pending_file_size);
        pendingFile.write((char*) record, sizeof(pending_order));
    } else {
        auto* tmp = pending_cache.get(tra->ticket_end);
        tmp->nxt = rt;
        pending_cache.set_dirty_bit(tra->ticket_end);
        tra->ticket_end = rt;
        pendingFile.seekp(pending_file_size);
        pendingFile.write((char*) record, sizeof(pending_order));
    }
    pending_file_size += sizeof(record);
}
void PendingTicketManager::cancel_pending(int order_key,train* ptr) {
    for (DiskLoc_T la = -1, where = ptr->ticket_head;;) {
//        assert(where);
        auto* p = pending_cache.get(where);
        if (p->key == order_key) {
            //delete p
            if (where == ptr->ticket_head)ptr->ticket_head = p->nxt;
            if (where == ptr->ticket_end)ptr->ticket_end = la;
            auto* la_p = pending_cache.get(la);
            la_p->nxt = p->nxt;
            pending_cache.set_dirty_bit(la);
            pending_cache.remove(where);
            break;
        }
        if (where == ptr->ticket_end)break;
        else la = where, where = p->nxt;
    }

}


