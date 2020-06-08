//
// Created by vortox on 28/3/20.
//

#ifndef TICKETSYSTEM_USERMANAGER_H
#define TICKETSYSTEM_USERMANAGER_H

#include <fstream>
#include <iostream>
#include "structure.h"
#include "UserOrderManager.h"
#include "../basic_component/SLRU.h"
#include "../basic_component/LRUBPtree.h"

using std::endl;
namespace t_sys {
    class UserManager {
    private:
        std::fstream userFile;
        DiskLoc_T user_file_size;
        int is_null;
        std::ostream& defaultOut;
        cache::LRUCache<DiskLoc_T, user> user_cache;
        ds::unordered_map<username_t, int> onlinePool; // username -> privilege
        bptree::LRUBPTree<username_t, DiskLoc_T> usernameToOffset;

        static void loadUser(std::fstream& ifs, DiskLoc_T offset, user* usr) {
            ifs.seekg(offset);
            ifs.read((char*) usr, sizeof(user));
        }
        static void saveUser(std::fstream& ofs, DiskLoc_T offset, const user* usr) {
            ofs.seekp(offset);
            ofs.write((char*) usr, sizeof(user));
        }
        DiskLoc_T increaseFile(const user* usr) {
            DiskLoc_T rt = user_file_size;
            userFile.seekp(user_file_size);
            userFile.write((char*) usr, sizeof(user));
            user_file_size += sizeof(user);
            return rt;
        }

    public:
        bool isOnline(const username_t& user) const;

        std::pair<bool,order> refund_and_return_order(UserOrderManager* ord_manager, const username_t& user, int x);

        std::pair<DiskLoc_T,int> addorder(UserOrderManager* ord_manager, const username_t& user, const order* record);

        int getPrivilege(const username_t& user);

        bool privilegeCompare(const username_t& origin, const username_t& target);

        bool Login(const username_t& user, const char* passwd);

        bool Logout(const username_t& user);

        bool Query_profile(const username_t& origin, const username_t& target);

        bool Modify_profile(const username_t& origin, const username_t& target,
                            const char* n_passed, const char* n_name, const char* n_mail, const int* n_privilege);

        bool Add_user(UserOrderManager* ord_manager, const username_t* cur_user, const username_t* u,
                      const char* passwd, const char* name, const char* mailaddr, int privilege);

        bool Query_Order(UserOrderManager* order_mgr, const username_t& usr);

        UserManager(const std::string& file_path, const std::string& username_index_path);

        ~UserManager();

        static void Init(const std::string& path) ;
    };
}
#endif //TICKETSYSTEM_USERMANAGER_H
