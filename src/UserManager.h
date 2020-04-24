//
// Created by vortox on 28/3/20.
//

#ifndef TICKETSYSTEM_USERMANAGER_H
#define TICKETSYSTEM_USERMANAGER_H

#include <fstream>
#include <iostream>
#include "structure.h"
#include "OrderManager.h"
#include "../include/cache.h"
#include "../include/LRUBPtree.h"
using std::endl;
namespace t_sys{
    class UserManager{
    private:
        std::fstream userFile;
        DiskLoc_T file_size;
        std::ostream& defaultOut;
        cache::LRUCache<DiskLoc_T,user> cache;
        ds::unordered_map<username_t,int> onlinePool; // username -> privilege
        bptree::LRUBPTree<username_t,DiskLoc_T> usernameToOffset;

        static void loadUser(std::fstream& ifs,DiskLoc_T offset,user* usr){
            ifs.seekg(offset);
            ifs.read((char*)usr, sizeof(user));
        }
        static void saveUser(std::fstream& ofs,DiskLoc_T offset,const user* usr){
            ofs.seekp(offset);
            ofs.write((char*)usr, sizeof(user));
        }
        DiskLoc_T increaseFile(const user* usr){
            DiskLoc_T rt=file_size;
            userFile.seekp(file_size);
            userFile.write((char*)usr, sizeof(user));
            file_size += sizeof(user);
            return rt;
        }

        void create(const std::string& path){
            std::fstream f(path,ios::out|ios::binary);
            char buf[sizeof(DiskLoc_T)];
            char* ptr = buf;
            DiskLoc_T sz=sizeof(DiskLoc_T);
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
            write_attribute(sz);
#undef write_attribute
            f.write(buf,sizeof(buf));
            f.close();
        }
    public:
        bool isOnline(const username_t& user) const {
            return onlinePool.find(user)!=onlinePool.end();
        }
        int getPrivilege(const username_t& user) {
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
        bool privilegeCompare(const username_t& origin, const username_t& target){
            // do not check online
            // return true implies that both users exist while not necessarily online
            int p=getPrivilege(origin),q=getPrivilege(target);
            return (p==-1||q==-1)? false:p>=q;
        }
        bool Login(const username_t& user,const char* passwd){
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
        bool Logout(const username_t& user){
            if(onlinePool.find(user)==onlinePool.end()){
                defaultOut<<"-1"<<endl;
                return false;
            } else{
                onlinePool.erase(user);
                defaultOut<<"0"<<endl;
                return true;
            }
        }
        bool Query_profile(const username_t& origin, const username_t& target){
            if(!(isOnline(origin)&&privilegeCompare(origin,target))){
                defaultOut<<"-1"<<endl;
                return false;
            }
            DiskLoc_T loc=usernameToOffset.search(target).first;
            auto* ptr=cache.get(loc);
            defaultOut<<(ptr->username.name)<<' '<<(ptr->name)<<' '<<(ptr->mailAddr)<<' '<<(ptr->privilege)<<endl;
            return true;
        }
        bool Modify_profile(const username_t& origin,const username_t& target,
                const char* n_passed,const char* n_name,const char* n_mail,const int* n_privilege){
            if(!(isOnline(origin)&&privilegeCompare(origin,target))){
                defaultOut<<"-1"<<endl;
                return false;
            }
            DiskLoc_T loc=usernameToOffset.search(target).first;
            auto* ptr=cache.get(loc);
            if(n_passed)
                strcpy(ptr->password,n_passed);
            if(n_name)
                strcpy(ptr->name,n_name);
            if(n_mail)
                strcpy(ptr->mailAddr,n_mail);
            if(n_privilege)
                ptr->privilege=*n_privilege;
            cache.dirty_bit_set(loc);
            defaultOut<<(ptr->username.name)<<' '<<(ptr->name)<<' '<<(ptr->mailAddr)<<' '<<(ptr->privilege)<<endl;
            return true;
        }
        bool Add_user(OrderManager* ord_manager, const username_t* cur_user, const username_t* u,
                      const char* passwd, const char* name, const char* mailaddr, int privilege){
            if(cur_user){

                int cur_pri=getPrivilege(*cur_user);
                if(cur_pri==-1||cur_pri<privilege){
                    defaultOut<<"-1"<<endl;
                    return false;
                }
            } else privilege=10;
            user usr{};
            strcpy(usr.username.name,u->name);
            strcpy(usr.password,passwd);
            strcpy(usr.name,name);
            strcpy(usr.mailAddr,mailaddr);
            usr.privilege=privilege;
            usr.orderOffset=ord_manager->createRecord();
            DiskLoc_T off=increaseFile(&usr);
            usernameToOffset.insert(*u,off);
            defaultOut<<"0"<<endl;
            return true;
        }
        UserManager(const std::string& file_path,const std::string& username_index_path,bool create_flag=false)
            :cache(51,[this](DiskLoc_T off,user* usr){loadUser(userFile,off,usr);},
                    [this](DiskLoc_T off,const user* usr){saveUser(userFile,off,usr);}),
                    usernameToOffset(username_index_path,107,create_flag),defaultOut(std::cout)
                    {
            if(create_flag)create(file_path);
            userFile.open(file_path);
            char buf[sizeof(file_size)];
            char* ptr = buf;
            userFile.seekg(0);
            userFile.read(buf, sizeof(buf));
#define read_attribute(ATTR) memcpy((void*)&ATTR,ptr,sizeof(ATTR));ptr+=sizeof(ATTR)
            read_attribute(file_size);
#undef read_attribute
        }

        ~UserManager(){
            char buf[sizeof(file_size)];
            char* ptr = buf;
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
            write_attribute(file_size);
#undef write_attribute
            userFile.seekg(0);
            userFile.write(buf, sizeof(buf));
            cache.destruct();
            userFile.close();
        }
    };
}
#endif //TICKETSYSTEM_USERMANAGER_H
