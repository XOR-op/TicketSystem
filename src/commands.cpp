#include "commands.h"
#include <sstream>
using std::cin;
using std::istringstream;
using namespace t_sys;
#define RANGE(n) for(int _HIDDEN_VA=0;_HIDDEN_VA<n;++_HIDDEN_VA)
void t_sys::query_profile(vars v){
    username_t cur,usr;
    RANGE(2){
        char c=getOption();
        if(c=='c')
            cin>>cur.name;
        else cin>>usr.name;
    }
    v.user_mgr.Query_profile(cur,usr);
}

void t_sys::query_ticket(vars v){
    char buf[500];
    char date[8],p[8],start[24],to[24];
    p[0]='t';
    cin.getline(buf,500);
    istringstream iss(buf);
    while (iss){
        switch (getOption(iss)) {
            case 's':iss>>start;break;
            case 't':iss>>to;break;
            case 'd':iss>>date;break;
            case 'p':iss>>p;break;
        }
    }
    v.train_mgr.Query_ticket(start,to,TrainManager::parsingDate(date),p[0]=='t'?TrainManager::TIME:TrainManager::COST);
}

void t_sys::query_transfer(vars v){
    char buf[500];
    char date[8],p[8],start[24],to[24];
    p[0]='t';
    cin.getline(buf,500);
    istringstream iss(buf);
    while (iss){
        switch (getOption(iss)) {
            case 's':iss>>start;break;
            case 't':iss>>to;break;
            case 'd':iss>>date;break;
            case 'p':iss>>p;break;
        }
    }
    v.train_mgr.Query_transfer(start,to,TrainManager::parsingDate(date),p[0]=='t'?TrainManager::TIME:TrainManager::COST);
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
    istringstream iss(buf);
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
    username_t username;
    getOption(); // must be '-u'
    cin>>username.name;
    v.user_mgr.Query_Order(&v.order_mgr,username);
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


void t_sys::release_train(vars v){
    trainID_t train_id;
    getOption();
    cin>>train_id.ID;
    v.train_mgr.Release_train(train_id);
}

void t_sys::delete_train(vars v){
    trainID_t train_id;
    getOption();
    cin>>train_id.ID;
    v.train_mgr.Delete_train(train_id);
}

void t_sys::query_train(vars v){
    trainID_t tid;
    char date[8];
    RANGE(2){
        char c=getOption();
        if(c=='i')
            cin>>tid.ID;
        else cin>>date;
    }
    v.train_mgr.Query_train(tid,TrainManager::parsingDate(date));
}

void t_sys::refund_ticket(vars v){
    char buf[64];
    username_t u;int n=1;
    cin.getline(buf,64);
    istringstream iss(buf);
    while (iss){
        switch (getOption(iss)) {
            case 'u':iss>>u.name;break;
            case 'n':iss>>n;break;
        }
    }
    // todo invoke a future function
}

void t_sys::buy_ticket(vars v){

}

void t_sys::add_train(vars v){

}
