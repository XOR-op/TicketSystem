//
// Created by vortox on 28/3/20.
//

#ifndef TICKETSYSTEM_USERMANAGER_H
#define TICKETSYSTEM_USERMANAGER_H

#include <fstream>
#include <iostream>
#include "structure.h"
#include "OrderManager.h"
#include "../bptree/cache.h"
#include "../bptree/LRUBPtree.h"
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
        bool isOnline(const username_t& user) const;
        int getPrivilege(const username_t& user);
        bool privilegeCompare(const username_t& origin, const username_t& target);
        bool Login(const username_t& user,const char* passwd);
        bool Logout(const username_t& user);
        bool Query_profile(const username_t& origin, const username_t& target);
        bool Modify_profile(const username_t& origin,const username_t& target,
                            const char* n_passed,const char* n_name,const char* n_mail,const int* n_privilege);
        bool Add_user(OrderManager* ord_manager, const username_t* cur_user, const username_t* u,
                      const char* passwd, const char* name, const char* mailaddr, int privilege);
        UserManager(const std::string& file_path,const std::string& username_index_path,bool create_flag=false);
        ~UserManager();
    };
}
#endif //TICKETSYSTEM_USERMANAGER_H
