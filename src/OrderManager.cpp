#include "OrderManager.h"
using namespace t_sys;

int OrderManager::getRecord(DiskLoc_T* where, order* ptr) {
    int size;
    DiskLoc_T cur=*where;
    static auto rd=[&cur,this](void* ptr,size_t size){file.read((char*)ptr,cur,size);cur+=size;};
    rd( where, sizeof(DiskLoc_T));
    rd(&size, sizeof(int));
    rd(ptr, DATA_SIZE*size);
    return size;
}

DiskLoc_T OrderManager::appendRecord(DiskLoc_T where, const order* record) {
    int size;
    file.read((char*) &size,where+sizeof(DiskLoc_T), sizeof(int));
    if (size == count) {
        // extend
        return extend(record, where);
    } else {
        file.write((char*) record,where+sizeof(int)+sizeof(DiskLoc_T)+DATA_SIZE*size, DATA_SIZE);
        size+=1;
        file.write((char*) &size,where+sizeof(DiskLoc_T), sizeof(int));
        return where;
    }
}

DiskLoc_T OrderManager::createRecord() {
    return extend(nullptr, NO_NEXT);
}

void OrderManager::printAllOrders(std::ostream& ofs,DiskLoc_T head){
    order buf[count];
    while (head!=NO_NEXT) {
        int size = getRecord(&head, buf);
        for(int i=size-1;i>=0;--i){
            auto& ref=buf[i];
            ofs<<(ref.stat==order::SUCCESS?"[success]":(ref.stat==order::PENDING?"[pending]":"[refunded]"))
               <<' '<<ref.trainID<<' '<<ref.from<<' '<<
               // todo design output format
        }
    }

}

OrderManager::OrderManager(const std::string& file_path, bool create_flag):file(file_path) {
    if (create_flag)initialize();
    // read metadata
    char buf[sizeof(file_size)];
    char* ptr = buf;
    file.read(buf,0, sizeof(buf));
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
    file.write(buf,0, sizeof(buf));
}

