#include <iostream>
#include <string>
#include <fstream>
#include <unordered_set>
#include "../src/UserManager.h"
#include "../src/structure.h"
using namespace std;
using namespace t_sys;
struct Tester {
    vector<user> us;
    OrderManager orm;
    unordered_set<int> index_set;
    UserManager* manager;
    username_t admin{"ADMIN"};
    void read() {
        fstream data("../testData/users.data");
        user u;
         while( data >> u.username.name >> u.password >> u.name >> u.mailAddr >> u.privilege){
            us.push_back(u);
        }

    }
    void inserts(){
        for(auto& u:us)
            manager->Add_user(&orm, &admin, &u.username, u.password, u.name, u.mailAddr, u.privilege);

    }
    void bound(UserManager* ma){
        manager=ma;
        manager->Add_user(&orm, nullptr, &admin, "passwd", "admin", "None", 0);
        manager->Login(admin,"passwd");

    }
    void test(){
        int a=9;
        manager->Modify_profile(admin,{"Bob5"},nullptr,"OYSM", nullptr,&a);
        manager->Query_profile(admin,admin);
        for(auto& u:us){
//            manager->Query_profile(admin,u.username);
        }
        manager->Logout(admin);
        manager->Query_profile(admin,admin);
        manager->Login(admin,"passwd");
        manager->Query_profile(admin,admin);
    }
} tester;
int main() {
    const string path("../experiDir/users.db");
    const string idx_path("../experiDir/users_idx.db");
    system(("test -e "+path+" && rm "+path).c_str());
    system(("test -e "+idx_path+" && rm "+idx_path).c_str());
    UserManager manager(path,idx_path, true);
    tester.bound(&manager);
    tester.read();
    tester.inserts();
    tester.test();
    return 0;
}