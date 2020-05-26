#include "OrderManager.h"
using namespace t_sys;
/*
 * fill file with record and return the block's offset
 */
DiskLoc_T OrderManager::extend(const order* record, DiskLoc_T where) {
    // construct buffer
    char buffer[sizeof(DiskLoc_T)+sizeof(int)+DATA_SIZE];
    char* buf = buffer;
    int size = (record != nullptr);
    assert(size>=0&&size<=1);
# define write_attribute(ATTR) do{memcpy(buf,(void*)&ATTR,sizeof(ATTR));buf+=sizeof(ATTR);}while(0)
    write_attribute(where);
    write_attribute(size);
    if (record) {
        write_attribute(*record);
    }
#undef write_attribute
    // write buffer
    file.seekp(file_size);
    file.write(buffer, sizeof(buffer));
    where = file_size;
    file_size += sizeof(DiskLoc_T)+sizeof(int)+DATA_SIZE*_order_block::COUNT;
    return where;
}

_order_block* OrderManager::getRecord(DiskLoc_T where) {
    return cache.get(where);
}

std::pair<DiskLoc_T,int> OrderManager::appendRecord(DiskLoc_T where, const order* record,int* offset_val) {
    auto* ptr=cache.get(where);
    if (ptr->size == _order_block::COUNT) {
        // extend
        return {extend(record, where),1};
    } else {
        ptr->data[ptr->size]=*record;
        ptr->size+=1;
        cache.set_dirty_bit(where);
        return {where,ptr->size};
    }
}

DiskLoc_T OrderManager::createRecord() {
    return extend(nullptr, NO_NEXT);
}
static const char* express(char* buf,int what){
    if(what==order::NONE_TIME){
        strcpy(buf,"xx-xx xx:xx");
    } else {
        assert(what<=99999999);
        buf[0] = '0'+(what/10000000)%10;
        buf[1] = '0'+(what/1000000)%10;
        buf[2] = '-';
        buf[3] = '0'+(what/100000)%10;
        buf[4] = '0'+(what/10000)%10;
        buf[5] = ' ';
        buf[6] = '0'+(what/1000)%10;
        buf[7] = '0'+(what/100)%10;
        buf[8] = ':';
        buf[9] = '0'+(what/10)%10;
        buf[10] = '0'+what%10;
        buf[11] = '\0';
    }
    return buf;
}
void OrderManager::printAllOrders(std::ostream& ofs,DiskLoc_T head){
    char str_buf[15];
    while (head!=NO_NEXT) {
        auto*ptr= getRecord(head);
        for(int i=ptr->size-1;i>=0;--i){
            auto& ref=ptr->data[i];
            ofs<<(ref.stat==order::SUCCESS?"[success]":(ref.stat==order::PENDING?"[pending]":"[refunded]"))
               <<' '<<ref.trainID<<' '<<ref.from<<' '<<express(str_buf,ref.leaveTime)<<" -> "<<ref.to
               <<' '<<express(str_buf,ref.arriveTime)<<' '<<ref.price<<' '<<ref.num<<std::endl;
        }
        head=ptr->nextOffset;
    }
}

OrderManager::OrderManager(const std::string& file_path):cache(107,
        [this](DiskLoc_T where,_order_block* blk){readBlock(file,where,blk);},
        [this](DiskLoc_T where,const _order_block* blk){writeBlock(file,where,blk);}){
    file.open(file_path,ios::binary|ios::in|ios::out);
    // read metadata
    char buf[sizeof(file_size)];
    char* ptr = buf;
    file.seekg(0);
    file.read(buf,sizeof(buf));
#define read_attribute(ATTR) memcpy((void*)&ATTR,ptr,sizeof(ATTR));ptr+=sizeof(ATTR)
    read_attribute(file_size);
#undef read_attribute
}

OrderManager::~OrderManager() {
    char buf[sizeof(file_size)];
    char* ptr = buf;
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
    write_attribute(file_size);
#undef write_attribute
    file.seekp(0);
    file.write(buf,sizeof(buf));
    cache.destruct();
    file.close();
}
std::pair<bool,order*> OrderManager::refundOrder(DiskLoc_T head, int n) {
    int cnt=0;
    while (head!=NO_NEXT){
        auto* ptr=cache.get(head);
        if(cnt+ptr->size>=n){
            // have found
            auto& ref=ptr->data[ptr->size-n+cnt];
            if(ref.stat!=order::REFUNDED){
                order tmp=ref;
                ref.stat=order::REFUNDED;
                cache.set_dirty_bit(head);
                return std::make_pair(true,&tmp);
            } else return std::make_pair(false, nullptr);
        }
        cnt+=ptr->size;
        head=ptr->nextOffset;
    }
    return std::make_pair(false, nullptr);
}

void OrderManager::setSuccess(DiskLoc_T block, int offset_in_block) {
    auto* ptr=cache.get(block);
    ptr->data[offset_in_block].stat=order::SUCCESS;
    cache.set_dirty_bit(block);
}

void OrderManager::Init(const std::string& path) {
    std::fstream file(path, ios::binary | ios::out);
    DiskLoc_T sz = sizeof(DiskLoc_T);
    file.write((char*) &sz, sizeof(sz));
    file.close();
}

void OrderManager::readBlock(std::fstream& ifs, DiskLoc_T where, _order_block* ptr) {
    ifs.seekg(where);
    ifs.read((char*) ptr,sizeof(_order_block));
}

void OrderManager::writeBlock(std::fstream& ofs, DiskLoc_T where,const _order_block* ptr) {
    ofs.seekp(where);
    ofs.write((const char*)ptr,sizeof(_order_block));
}
