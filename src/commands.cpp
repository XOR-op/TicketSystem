#include "commands.h"
#include <sstream>
using std::cin;
using namespace t_sys;
#define RANGE(n) for(int _HIDDEN_VA=0;_HIDDEN_VA<n;++_HIDDEN_VA)
void t_sys::query_profile(vars v){

}

void t_sys::query_ticket(vars v){

}

void t_sys::buy_ticket(vars v){

}

void t_sys::login(vars v){
    username_t username;
    char passwd[32],c;
    RANGE(2) {
        c = getOption();
        if (c == 'u') {
            cin >> username.name;
        } else cin>>passwd;
    }
    v.user_mgr.Login(username,passwd);
}

void t_sys::logout(vars v){
    username_t username;
    getOption(); // must be '-u'
    cin>>username.name;
    v.user_mgr.Logout(username);
}

void t_sys::modify_profile(vars v){
    char buf[500];
    cin.getline(buf,500);
    std::istringstream iss(buf);
    username_t cur;
    user u;
    char *passwd= nullptr,*mail= nullptr,*name= nullptr,c;
    int p=-1;
    while (iss){
        c=getOption(iss);
        switch (c) {
            case 'c':iss>>cur.name;break;
            case 'u':iss>>u.username.name;break;
            case 'p':iss>>u.password;passwd=u.password;break;
            case 'n':iss>>u.name;name=u.name;break;
            case 'm':iss>>u.mailAddr;mail=u.mailAddr;break;
            case 'g':iss>>p;break;
        }
    }
    v.user_mgr.Modify_profile(cur,u.username,passwd,name,mail,p==-1? nullptr:&p);
}

void t_sys::query_order(vars v){

}

void t_sys::add_user(vars v){
    user u;
    username_t cur;
    RANGE(6){
        switch (getOption()) {
            case 'c':cin>>cur.name;break;
            case 'u':cin>>u.username.name;break;
            case 'p':cin>>u.password;break;
            case 'n':cin>>u.name;break;
            case 'm':cin>>u.mailAddr;break;
            case 'g':cin>>u.privilege;break;
        }
    }
    v.user_mgr.Add_user(&v.order_mgr,&cur,&u.username,u.password,u.name,u.mailAddr,u.privilege);
}

void t_sys::add_train(vars v){

}

void t_sys::release_train(vars v){
    trainID_t train_id;
    getOption();
    cin>>train_id.ID;
    v.train_mgr.Release_train(train_id);
}

void t_sys::query_train(vars v){

}

void t_sys::delete_train(vars v){

}

void t_sys::query_transfer(vars v){

}

void t_sys::refund_ticket(vars v){

}


