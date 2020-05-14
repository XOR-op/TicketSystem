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
static const char* express(char* buf,int what){
    if(what==order::NONE_TIME){
        strcpy(buf,"xx-xx xx:xx");
    } else {
        buf[0] = '0'+(what/10000000)%10;
        buf[1] = '0'+(what/10000000)%10;
        buf[2] = '-';
        buf[3] = '0'+(what/10000000)%10;
        buf[4] = '0'+(what/10000000)%10;
        buf[5] = ' ';
        buf[6] = '0'+(what/10000000)%10;
        buf[7] = '0'+(what/10000000)%10;
        buf[8] = ':';
        buf[9] = '0'+(what/10000000)%10;
        buf[10] = '0'+(what/10000000)%10;
        buf[11] = '\0';
    }
    return buf;
}
void OrderManager::printAllOrders(std::ostream& ofs,DiskLoc_T head){
    order buf[count];
    char str_buf[15];
    while (head!=NO_NEXT) {
        int size = getRecord(&head, buf);
        for(int i=size-1;i>=0;--i){
            auto& ref=buf[i];
            ofs<<(ref.stat==order::SUCCESS?"[success]":(ref.stat==order::PENDING?"[pending]":"[refunded]"))
               <<' '<<ref.trainID<<' '<<ref.from<<' '<<express(str_buf,ref.leaveTime)<<" -> "<<ref.to
               <<' '<<express(str_buf,ref.arriveTime)<<' '<<ref.price<<' '<<ref.num<<std::endl;
        }
    }
}

OrderManager::OrderManager(const std::string& file_path):file(file_path) {
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
bool OrderManager::refundOrder(DiskLoc_T head, int n) {
    order buf[count];
    int cnt=0;
    while (head!=NO_NEXT){
        int sz=getRecord(&head,buf);
        if(cnt+sz>=n){
            // have found
            auto& ref=buf[sz-n+cnt];
            if(ref.stat!=order::REFUNDED){
                order::STATUS stat=order::REFUNDED;
                file.write((char*)&stat,head+sizeof(order)*(sz-n+cnt),sizeof(stat));
                return true;
            } else return false;
        }
        cnt+=sz;
    }
    return false;
}

