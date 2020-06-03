#include <iostream>
#include <fstream>
#include <cstdio>
#include "global.h"
#include "../basic_component/LRUBPtree.h"
#include "../include/SerializationHelper.h"
#include "structure.h"
#include "UserManager.h"
#include "TrainManager.h"
#include "UserOrderManager.h"
#include "main_helper.h"
#include "commands.h"
//#include "../include/debug.h"

using namespace std;
using namespace t_sys;
using namespace bptree;
//Debug::CacheMissRater LRUrater,SLRUrater;
//using Debug::SLRUrater;
void init(){
    ds::SerialVector<trainID_t>::Init(TRAIN_INFO_PATH);
    ds::SerialMap<station_t,int>::Init(STATION_INFO_PATH);
    LRUBPTree<trainID_t, DiskLoc_T>::Init(TRAIN_TRAIN_ID_INDEX_PATH);
    LRUBPTree<long long, int>::Init(TRAIN_STATION_INDEX_PATH);
    LRUBPTree<username_t, DiskLoc_T>::Init(USER_INDEX_PATH);
    PendingTicketManager::Init(PENDING_PATH);
    UserManager::Init(USER_PATH);
    TrainManager::Init(TRAIN_PATH);
    UserOrderManager::Init(ORDER_PATH);
}

void cleanAll(){
    remove(TRAIN_INFO_PATH);
    remove(STATION_INFO_PATH);
    remove(TRAIN_STATION_INDEX_PATH);
    remove(TRAIN_TRAIN_ID_INDEX_PATH);
    remove(PENDING_PATH);
    remove(USER_INDEX_PATH);
    remove(USER_PATH);
    remove(TRAIN_PATH);
    remove(ORDER_PATH);
}
int instance(){
    if (needInit()) init();
    auto* user_mgr=new UserManager(USER_PATH, USER_INDEX_PATH);
    auto* train_mgr= new TrainManager(TRAIN_PATH, TRAIN_TRAIN_ID_INDEX_PATH, TRAIN_STATION_INDEX_PATH, TRAIN_INFO_PATH,
                                      STATION_INFO_PATH);
    auto* order_mgr=new UserOrderManager(ORDER_PATH);
    auto* pending_mgr=new PendingTicketManager(PENDING_PATH);
    vars binding(train_mgr,user_mgr,order_mgr,pending_mgr);
    char buffer[16];
    while (cin>>buffer){
        if(buffer[0]=='q'){
            if(buffer[6]=='p'){
                // query_profile
                query_profile(binding);
            } else if(buffer[6]=='o'){
                // query_order
                query_order(binding);
            } else{
                if(buffer[7]=='i'){
                    // query_ticket
                    query_ticket(binding);
                } else if(buffer[9]=='i'){
                    // query_train
                    query_train(binding);
                } else{
                    // query_transfer
                    query_transfer(binding);
                }
            }
        } else if(buffer[0]=='a'){
            if(buffer[4]=='u'){
                // add_user
                add_user(binding);
            } else{
                // add_train
                add_train(binding);
            }
        } else if(buffer[0]=='l'){
            if(buffer[3]=='i'){
                // login
                login(binding);
            } else{
                // logout
                logout(binding);
            }
        } else if(buffer[0]=='r'){
            if(buffer[2]=='l'){
                // release_train
                release_train(binding);
            } else{
                // refund_ticket
                refund_ticket(binding);
            }
        } else if(buffer[0]=='e'){
            // exit
            cout<<"bye"<<endl;
            break;
        }
        else{
            switch (buffer[0]) {
                case 'm':
                    // modify_profile
                    modify_profile(binding);
                    break;
                case 'b':
                    // buy_ticket
                    buy_ticket(binding);
                    break;
                case 'c':
                    // clean
                    cleanAll();
                    delete user_mgr;
                    delete train_mgr;
                    delete order_mgr;
                    delete pending_mgr;
                    init();
                    user_mgr=new UserManager(USER_PATH, USER_INDEX_PATH);
                    train_mgr= new TrainManager(TRAIN_PATH, TRAIN_TRAIN_ID_INDEX_PATH, TRAIN_STATION_INDEX_PATH,TRAIN_INFO_PATH,STATION_INFO_PATH);
                    order_mgr=new UserOrderManager(ORDER_PATH);
                    pending_mgr=new PendingTicketManager(PENDING_PATH);
                    binding=vars(train_mgr,user_mgr,order_mgr,pending_mgr);
                    cout<<"0"<<endl;
                    break;
                case 'd':
                    // delete_train
                    delete_train(binding);
                    break;
            }
        }
    }
    {
        // debug
#ifdef DEBUG_FLAG
        double vm,rss;
        Debug::process_mem_usage(vm,rss);
        cout<<"VM: "<<vm<<"; RSS: "<<rss<<endl;
        if(SLRUrater.good()){
            SLRUrater.hitRate();
            SLRUrater.hotByHit();
            SLRUrater.coldByHit();
        }
        if(LRUrater.good()){
            LRUrater.hitRate();
        }
#endif
    }
    delete user_mgr;
    delete train_mgr;
    delete order_mgr;
    delete pending_mgr;
    return 0;

}
int main() {
#define NDEBUG
#ifndef NDEBUG
    // debug only
    cleanAll();
    for(int i=1;i<=5;++i) {
        std::ifstream ifs("../testData/data1/data/"+to_string(i)+".in");
        cin.rdbuf(ifs.rdbuf());
        instance();
    }
#else
    instance();
#endif
    return 0;
}
