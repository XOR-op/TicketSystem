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
    v.user_mgr->Query_profile(cur,usr);
}

void t_sys::query_ticket(vars v){
    char date[8],p[8],start[24],to[24];
    p[0]='t';
    while (cin.get()!='\n'){
        switch (getOption()) {
            case 's':cin >> start;break;
            case 't':cin >> to;break;
            case 'd':cin >> date;break;
            case 'p':cin >> p;break;
        }
    }
    v.train_mgr->Query_ticket(start,to,TrainManager::parsingDate(date),p[0]=='t'?TrainManager::TIME:TrainManager::COST);
}

void t_sys::query_transfer(vars v){
    char date[8],p[8],start[24],to[24];
    p[0]='t';
    while (cin.get()!='\n'){
        switch (getOption()) {
            case 's':cin >> start;break;
            case 't':cin >> to;break;
            case 'd':cin >> date;break;
            case 'p':cin >> p;break;
        }
    }
    v.train_mgr->Query_transfer(start,to,TrainManager::parsingDate(date),p[0]=='t'?TrainManager::TIME:TrainManager::COST);
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
    v.user_mgr->Login(username,passwd);
}

void t_sys::logout(vars v){
    username_t username;
    getOption(); // must be '-u'
    cin>>username.name;
    v.user_mgr->Logout(username);
}

void t_sys::modify_profile(vars v){
    username_t cur;
    user u;
    char *passwd= nullptr,*mail= nullptr,*name= nullptr;
    int p=-1;
    while (cin.get()!='\n'){
        switch (getOption()) {
            case 'c':cin >> cur.name;break;
            case 'u':cin >> u.username.name;break;
            case 'p':cin >> u.password;passwd=u.password;break;
            case 'n':cin >> u.name;name=u.name;break;
            case 'm':cin >> u.mailAddr;mail=u.mailAddr;break;
            case 'g':cin >> p;break;
        }
    }
    v.user_mgr->Modify_profile(cur,u.username,passwd,name,mail,p==-1? nullptr:&p);
}

void t_sys::query_order(vars v){
    username_t username;
    getOption(); // must be '-u'
    cin>>username.name;
    v.user_mgr->Query_Order(v.order_mgr,username);
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
    v.user_mgr->Add_user(v.order_mgr,&cur,&u.username,u.password,u.name,u.mailAddr,u.privilege);
}


void t_sys::release_train(vars v){
    trainID_t train_id;
    getOption();
    cin>>train_id.ID;
    v.train_mgr->Release_train(train_id);
}

void t_sys::delete_train(vars v){
    trainID_t train_id;
    getOption();
    cin>>train_id.ID;
    v.train_mgr->Delete_train(train_id);
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
    v.train_mgr->Query_train(tid,TrainManager::parsingDate(date));
}

void t_sys::refund_ticket(vars v){
    username_t u;int n=1;
    while (cin.get()!='\n'){
        switch (getOption()) {
            case 'u':cin>>u.name;break;
            case 'n':cin>>n;break;
        }
    }
    v.train_mgr->Refund_ticket(v.user_mgr,v.order_mgr,u,n);
}

void t_sys::buy_ticket(vars v){
    username_t usr;
    trainID_t tid;
    char date[8],from[24],to[24],q[8];
    int n;
    q[0]='f';
    while (cin.get()!='\n'){
        switch (getOption()) {
            case 'u':cin>>usr.name;break;
            case 'i':cin>>tid.ID;break;
            case 'd':cin>>date;break;
            case 'n':cin>>n;break;
            case 'f':cin>>from;break;
            case 't':cin>>to;break;
            case 'q':cin>>q;break;
        }
    }
    v.train_mgr->Buy_ticket(v.user_mgr,v.order_mgr,usr,tid,TrainManager::parsingDate(date),n,from,to,q[0]=='t');
}

void splitInt(const char* buf,int* arr){
    int curN=0;
    for(const auto * ptr=buf;*ptr;++ptr){
        if(*ptr=='|') {
            *arr++ = curN;
            curN=0;
        }else curN=curN*10+*ptr-'0';
    }
    *arr=curN;
}

void splitStr(char* buf,char** arr){
    char* cur=buf;
    for(char* ptr=buf;*ptr;++ptr){
        if(*ptr=='|') {
            *arr++=cur;
            *ptr='\0';
            cur=ptr+1;
        }
    }
    *arr=cur;
}

int parsingDateAndTime(const char* str){
    return (str[0]-'0')*10000000+(str[1]-'0')*1000000+(str[3]-'0')*100000+(str[4]-'0')*10000+(str[6]-'0')*1000+(str[7]-'0')*100+(str[9]-'0')*10+(str[10]-'0');
}

void t_sys::add_train(vars v){
    char station_buf[21*101],prices_buf[7*101],travelTimes_buf[6*101],stopTimes_buf[6*101],start_time[8],salesDate_buf[16];
    trainID_t tid;
    int seatNum,stationNum;
    char type;
    while (cin.get()!='\n'){
        switch (getOption()) {
            case 'i':cin>>tid.ID;break;
            case 'n':cin>>stationNum;break;
            case 'm':cin>>seatNum;break;
            case 's':cin>>station_buf;break;
            case 'p':cin>>prices_buf;break;
            case 'x':cin>>start_time;break;
            case 't':cin>>travelTimes_buf;break;
            case 'o':cin>>stopTimes_buf;break;
            case 'd':cin>>salesDate_buf;break;
            case 'y':cin>>type;break;
        }
    }
    char* stations[101];
    int prices[101],travelTime[101],stopoverTimes[101],salesDate=parsingDateAndTime(salesDate_buf);
    splitStr(station_buf,stations);
    splitInt(travelTimes_buf,travelTime);
    splitInt(stopTimes_buf,stopoverTimes);
    v.train_mgr->Add_train(tid,stationNum,seatNum,stations,prices,TrainManager::parsingTime(start_time),travelTime,stopoverTimes,salesDate,type);
}
