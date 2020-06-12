#include "UserOrderManager.h"
using namespace t_sys;
/*
 * fill file with record and return the block's offset
 */
DiskLoc_T UserOrderManager::extend(const order* record, DiskLoc_T nextOffset) {
    // construct buffer
//    char buffer[sizeof(DiskLoc_T)+sizeof(int)+DATA_SIZE];
    char buffer[sizeof(_order_block)];
    char* buf = buffer;
    int size = (record != nullptr);
    assert(size>=0&&size<=1);
# define write_attribute(ATTR) do{memcpy(buf,(void*)&ATTR,sizeof(ATTR));buf+=sizeof(ATTR);}while(0)
    write_attribute(nextOffset);
    write_attribute(size);
    if (record) {
        write_attribute(*record);
    }
#undef write_attribute
    // write buffer
    assert(file.good());
    file.seekp(order_file_size);
    assert(file.good());
    file.write(buffer, sizeof(buffer));
    assert(file.good());
    DiskLoc_T where = order_file_size;
    order_file_size += sizeof(_order_block);
    return where;
}

_order_block* UserOrderManager::getRecord(DiskLoc_T where) {
    return order_block_cache.get(where);
}

std::pair<DiskLoc_T,int> UserOrderManager::appendRecord(DiskLoc_T where, const order* record) {
    auto* block_ptr=order_block_cache.get(where);
    if (block_ptr->size == _order_block::COUNT) {
        // extend
        return {extend(record, where),0};
    } else {
        block_ptr->data[block_ptr->size]=*record;
        block_ptr->size+=1;
        order_block_cache.set_dirty_bit(where);
        return {where, block_ptr->size-1};
    }
}

DiskLoc_T UserOrderManager::createRecord() {
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
void UserOrderManager::printAllOrders(std::ostream& ofs, DiskLoc_T head){
    char str_buf[15];
    while (head!=NO_NEXT) {
        auto*ptr= getRecord(head);
        for(int i=ptr->size-1;i>=0;--i){
            auto& ref=ptr->data[i];
            ofs<<(ref.stat==order::SUCCESS?"[success]":(ref.stat==order::PENDING?"[pending]":"[refunded]"))
               <<' '<<ref.trainID<<' '<<ref.from<<' '<<express(str_buf,ref.leaveTime)<<" -> "<<ref.to
               <<' '<<express(str_buf,ref.arriveTime)<<' '<<ref.price<<' '<<ref.num<<endl;
        }
        head=ptr->nextOffset;
        assert(head<1073741824); // 1GB, should not occur in local test
    }
}

UserOrderManager::UserOrderManager(const std::string& file_path): order_block_cache(200,
                                                                                    [this](DiskLoc_T where,_order_block* blk){readBlock(file,where,blk);},
                                                                                    [this](DiskLoc_T where,const _order_block* blk){writeBlock(file,where,blk);}){
    file.open(file_path,ios::binary|ios::in|ios::out);
    // read metadata
    char buf[sizeof(order_file_size)];
    char* ptr = buf;
    file.seekg(0);
    file.read(buf,sizeof(buf));
#define read_attribute(ATTR) memcpy((void*)&ATTR,ptr,sizeof(ATTR));ptr+=sizeof(ATTR)
    read_attribute(order_file_size);
#undef read_attribute
}

UserOrderManager::~UserOrderManager() {
    char buf[sizeof(order_file_size)];
    char* ptr = buf;
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
    write_attribute(order_file_size);
#undef write_attribute
    assert(file.good());
    order_block_cache.destruct();
    file.seekp(0);
    file.write(buf,sizeof(buf));
    file.flush();
    file.close();
}
std::pair<bool,order> UserOrderManager::refundOrder(DiskLoc_T head, int n) {
    int cnt=0;
    while (head!=NO_NEXT){
        auto* block_ptr=order_block_cache.get(head);
        if(cnt+block_ptr->size >= n){
            // have found
            auto& ref=block_ptr->data[block_ptr->size-n+cnt];
            if(ref.stat!=order::REFUNDED){
                // origin order
                order origin_order=ref;
                ref.stat=order::REFUNDED;
                order_block_cache.set_dirty_bit(head);
                return std::make_pair(true, origin_order);
            } else return std::make_pair(false, order());
        }
        cnt+=block_ptr->size;
        head=block_ptr->nextOffset;
    }
    return std::make_pair(false, order());
}

void UserOrderManager::setSuccess(DiskLoc_T block, int offset_in_block) {
    auto* block_ptr=order_block_cache.get(block);
    assert(block_ptr->data[offset_in_block].stat==order::PENDING);
    block_ptr->data[offset_in_block].stat=order::SUCCESS;
    order_block_cache.set_dirty_bit(block);
}

void UserOrderManager::Init(const std::string& path) {
    std::fstream file(path, ios::binary | ios::out);
    DiskLoc_T sz = sizeof(DiskLoc_T);
    file.write((char*) &sz, sizeof(sz));
    file.close();
}

void UserOrderManager::readBlock(std::fstream& ifs, DiskLoc_T where, _order_block* ptr) {
    ifs.seekg(where);
    ifs.read((char*) ptr,sizeof(_order_block));
}

void UserOrderManager::writeBlock(std::fstream& ofs, DiskLoc_T where, const _order_block* ptr) {
    ofs.seekp(where);
    ofs.write((const char*)ptr,sizeof(_order_block));
}
