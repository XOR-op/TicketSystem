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
PendingTicketManager::PendingTicketManager(const std::string& path):pendingFile(path){
    char buf2[sizeof(pending_file_size)+sizeof(freelist_head)];
    char* ptr2 = buf2;
    pendingFile.read(buf2,0, sizeof(buf2));
#define read_attribute(ATTR) memcpy((void*)&ATTR,ptr2,sizeof(ATTR));ptr2+=sizeof(ATTR)
    read_attribute(pending_file_size);
    read_attribute(freelist_head);
#undef read_attribute

}
PendingTicketManager::~PendingTicketManager() {
    char buf[sizeof(pending_file_size)+sizeof(freelist_head)];
    char* ptr = buf;
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
    write_attribute(pending_file_size);
    write_attribute(freelist_head);
#undef write_attribute
    pendingFile.write(buf,0, sizeof(buf));
}
void PendingTicketManager::allocate_tickets(UserOrderManager* ord_manager, train* train_ptr, const order* Order) {
    // try to dequeue pending orders
    for (DiskLoc_T prev_ofst = -1, where = train_ptr->ticket_head; where != -1;) {
        assert(where); // offset 0 is reserved for file_size
        pending_order p;
        pendingFile.read((char*)&p,where,sizeof(pending_order));
//        auto* pending_o = pending_cache.get(where);
        auto* pending_o =&p;
        bool able_to_buy = false;
        if (pending_o->day == Order->day) {
            able_to_buy = true;
            for (int j = pending_o->s; j < pending_o->t; j++)
                if (pending_o->num > train_ptr->stationTicketRemains[pending_o->day][j]) {
                    able_to_buy = false;
                    break;
                }
        }
        if (able_to_buy) {
            for (int j = pending_o->s; j < pending_o->t; j++)
                train_ptr->stationTicketRemains[pending_o->day][j]-=pending_o->num;
            ord_manager->setSuccess(pending_o->block, pending_o->offset_in_block);
            // remove pending_o from linked list
            if (where == train_ptr->ticket_head)train_ptr->ticket_head = pending_o->nxt;
            if (where == train_ptr->ticket_end)train_ptr->ticket_end = prev_ofst;
            if(prev_ofst != -1) {
                // fix prev node
                pending_order prev;
                pendingFile.read((char*)&prev,prev_ofst,sizeof(pending_order));
                prev.nxt=pending_o->nxt;
                pendingFile.write((char*)&prev,prev_ofst,sizeof(pending_order));
            }
            auto backup=pending_o->nxt;
            add_free_block(where);
            where = backup;
        } else {
            prev_ofst = where, where = pending_o->nxt;
        }
    }
}
void PendingTicketManager::add_pendingorder(pending_order* record, train* tra) {
    DiskLoc_T allocated;
    if(freelist_head==NONE){
        allocated=pending_file_size;
        pending_file_size+=sizeof(pending_order);
    } else{
        allocated=freelist_head;
        pendingFile.read((char*)&freelist_head, allocated, sizeof(DiskLoc_T));
    }
    if (tra->ticket_head == -1) {
        tra->ticket_head = tra->ticket_end = allocated;
    } else {
        pending_order prev;
        pendingFile.read((char*)&prev,tra->ticket_end,sizeof(pending_order));
        prev.nxt=allocated;
        pendingFile.write((char*)&prev,tra->ticket_end,sizeof(pending_order));
        tra->ticket_end = allocated;
    }
    pendingFile.write((char*) record, allocated, sizeof(pending_order));
}
void PendingTicketManager::cancel_pending(int order_key,train* ptr) {
    for (DiskLoc_T la = -1, where = ptr->ticket_head;;) {
        assert(where);
        pending_order po;
        pendingFile.read((char*)&po,where,sizeof(pending_order));
        auto* p=&po;
        if (p->key == order_key) {
            //delete p
            if (where == ptr->ticket_head)ptr->ticket_head = p->nxt;
            if (where == ptr->ticket_end)ptr->ticket_end = la;
            pending_order prev;
            pendingFile.read((char*)&prev,la,sizeof(pending_order));
            prev.nxt=p->nxt;
            pendingFile.write((char*)&prev,la,sizeof(pending_order));
            add_free_block(where);
            break;
        }
        if (where == ptr->ticket_end)break;
        else la = where, where = p->nxt;
    }

}
void PendingTicketManager::Init(const std::string& path) {
    std::fstream f(path, ios::out | ios::binary);
    char buf[sizeof(DiskLoc_T)*2];
    char* ptr = buf;
    DiskLoc_T sz = sizeof(buf);
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
    write_attribute(sz);
    write_attribute(NONE);
#undef write_attribute
    f.write(buf, sizeof(buf));
    f.close();
}

void PendingTicketManager::add_free_block(DiskLoc_T where) {
    pendingFile.write((char*)&freelist_head,where,sizeof(DiskLoc_T));
    freelist_head=where;
}


