#include "UserManager.h"
using namespace t_sys;
bool UserManager::isOnline(const username_t& user) const {
    return onlinePool.find(user)!=onlinePool.end();
}
std::pair<bool,order*> UserManager::getorder(OrderManager* ord_manager,const username_t &user, int x) {
    DiskLoc_T loc=usernameToOffset.search(user).first;
    auto* ptr=cache.get(loc);
    return ord_manager->refundOrder(ptr->orderOffset,x);
}
std::pair<DiskLoc_T,int> UserManager::addorder(OrderManager* ord_manager,const username_t &user, const order *record) {
    DiskLoc_T loc=usernameToOffset.search(user).first;
    auto* ptr=cache.get(loc);
    auto where=ord_manager->appendRecord(ptr->orderOffset,record);
    ptr->orderOffset=where.first;
    cache.set_dirty_bit(loc);
    return where;
}
int UserManager::getPrivilege(const username_t& user) {
    auto iter=onlinePool.find(user);
    if(iter!=onlinePool.end()){
        return iter->second;
    } else{
        auto rt=usernameToOffset.search(user);
        if(rt.second){
            auto* ptr=cache.get(rt.first);
            return ptr->privilege;
        } else{
            // not found
            return -1;
        }
    }
}
bool UserManager::privilegeCompare(const username_t& origin, const username_t& target){
    // do not check online
    // return true implies that both users exist while not necessarily online
    int p=getPrivilege(origin),q=getPrivilege(target);
    return (p==-1||q==-1)? false:p>=q;
}
bool UserManager::Login(const username_t& user,const char* passwd){
    if(onlinePool.find(user)==onlinePool.end()){
        auto par=usernameToOffset.search(user);
        if(par.second) {
            auto* ptr = cache.get(par.first);
            if(!strcmp(ptr->password,passwd)){
                onlinePool[user]=ptr->privilege;
                defaultOut<<"0"<<endl;
                return true;
            }
        }
    }
    defaultOut<<"-1"<<endl;
    return false;
}
bool UserManager::Logout(const username_t& user){
    if(onlinePool.find(user)==onlinePool.end()){
        defaultOut<<"-1"<<endl;
        return false;
    } else{
        onlinePool.erase(user);
        defaultOut<<"0"<<endl;
        return true;
    }
}
bool UserManager::Query_profile(const username_t& origin, const username_t& target){
    if(!(isOnline(origin)&&privilegeCompare(origin,target))){
        defaultOut<<"-1"<<endl;
        return false;
    }
    DiskLoc_T loc=usernameToOffset.search(target).first;
    auto* ptr=cache.get(loc);
    defaultOut<<(ptr->username.name)<<' '<<(ptr->name)<<' '<<(ptr->mailAddr)<<' '<<(ptr->privilege)<<endl;
    return true;
}
bool UserManager::Modify_profile(const username_t& origin,const username_t& target,
                    const char* n_passed,const char* n_name,const char* n_mail,const int* n_privilege){
    if(!(isOnline(origin)&&privilegeCompare(origin,target))){
        defaultOut<<"-1"<<endl;
        return false;
    }
    DiskLoc_T loc=usernameToOffset.search(target).first;
    auto* ptr=cache.get(loc);
    if(n_privilege) {
        if(*n_privilege>=getPrivilege(origin)){
            defaultOut<<"-1"<<endl;
            return false;
        }
        ptr->privilege = *n_privilege;
    }
    if(n_passed)
        strcpy(ptr->password,n_passed);
    if(n_name)
        strcpy(ptr->name,n_name);
    if(n_mail)
        strcpy(ptr->mailAddr,n_mail);
    cache.set_dirty_bit(loc);
    defaultOut<<(ptr->username.name)<<' '<<(ptr->name)<<' '<<(ptr->mailAddr)<<' '<<(ptr->privilege)<<endl;
    return true;
}
bool UserManager::Add_user(OrderManager* ord_manager, const username_t* cur_user, const username_t* u,
              const char* passwd, const char* name, const char* mailaddr, int privilege){
    if(!is_null){
        int cur_pri=getPrivilege(*cur_user);
        if(!isOnline(*cur_user)||cur_pri==-1||cur_pri<=privilege||getPrivilege(*u)!=-1){
            // check -c
            defaultOut<<"-1"<<endl;
            return false;
        }
    } else {
        privilege=10;
        is_null= false;
    }
    // construct
    user usr{};
    strcpy(usr.username.name,u->name);
    strcpy(usr.password,passwd);
    strcpy(usr.name,name);
    strcpy(usr.mailAddr,mailaddr);
    usr.privilege=privilege;
    usr.orderOffset=ord_manager->createRecord();
    // write back
    DiskLoc_T off=increaseFile(&usr);
    usernameToOffset.insert(*u,off);
    defaultOut<<"0"<<endl;
    return true;
}
UserManager::UserManager(const std::string& file_path,const std::string& username_index_path)
        :cache(51,[this](DiskLoc_T off,user* usr){loadUser(userFile,off,usr);},
               [this](DiskLoc_T off,const user* usr){saveUser(userFile,off,usr);}),
         usernameToOffset(username_index_path,107),defaultOut(std::cout)
{
    userFile.open(file_path);
    if(userFile.bad())
        throw std::runtime_error("UserManger:file_path can't open");
    // metadata
    char buf[sizeof(file_size)+sizeof(is_null)];
    char* ptr = buf;
    userFile.seekg(0);
    userFile.read(buf, sizeof(buf));
#define read_attribute(ATTR) memcpy((void*)&ATTR,ptr,sizeof(ATTR));ptr+=sizeof(ATTR)
    read_attribute(file_size);
    read_attribute(is_null);
#undef read_attribute
}

UserManager::~UserManager(){
    // write metadata
    char buf[sizeof(file_size)+sizeof(int)];
    char* ptr = buf;
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
    write_attribute(file_size);
    write_attribute(is_null);
#undef write_attribute
    userFile.seekp(0);
    userFile.write(buf, sizeof(buf));
    cache.destruct();
    userFile.close();
}
bool UserManager::Query_Order(OrderManager* order_mgr, const username_t& usr) {
    if(isOnline(usr)){
        auto pair=usernameToOffset.search(usr);
        auto* ptr=cache.get(pair.first);
        order_mgr->printAllOrders(defaultOut,ptr->orderOffset);
        return true;
    } else{
        defaultOut<<"-1"<<endl;
        return false;
    }
}
void UserManager::Init(const std::string& path) {
        std::fstream f(path, ios::out | ios::binary);
        char buf[sizeof(DiskLoc_T)+sizeof(int)];
        char* ptr = buf;
        DiskLoc_T sz = sizeof(DiskLoc_T)+sizeof(int);
        int null_stat=true;
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
        write_attribute(sz);
        write_attribute(null_stat);
#undef write_attribute
        f.write(buf, sizeof(buf));
        f.close();
}


