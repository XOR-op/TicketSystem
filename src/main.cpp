#include <iostream>
#include <fstream>
#include <cstdio>
#include "global.h"
#include "../basic_component/LRUBPtree.h"
#include "structure.h"
#include "UserManager.h"
#include "TrainManager.h"
#include "OrderManager.h"
#include "main_helper.h"

using namespace std;
using namespace t_sys;
using namespace bptree;

void init(){
    LRUBPTree<trainID_t, DiskLoc_T>::Init(TRAIN_TRAIN_ID_INDEX_PATH);
    LRUBPTree<long long, int>::Init(TRIAN_STATION_INDEX_PATH);
    LRUBPTree<username_t, DiskLoc_T>::Init(USER_INDEX_PATH);
    UserManager::Init(USER_PATH);
    TrainManager::Init(TRAIN_PATH);
    OrderManager::Init(ORDER_PATH);
}

void cleanAll(){
    remove(TRIAN_STATION_INDEX_PATH);
    remove(TRAIN_TRAIN_ID_INDEX_PATH);
    remove(USER_INDEX_PATH);
    remove(USER_PATH);
    remove(TRAIN_PATH);
    remove(ORDER_PATH);
}

int main() {
    if (needInit()) init();
    UserManager user_mgr(USER_PATH, USER_INDEX_PATH);
    TrainManager train_mgr(TRAIN_PATH, TRAIN_TRAIN_ID_INDEX_PATH, TRIAN_STATION_INDEX_PATH);
    OrderManager order_mgr(ORDER_PATH);
    vars binding(train_mgr,user_mgr,order_mgr);
    char buffer[16];
    while (cin>>buffer){
        if(buffer[0]=='q'){
            if(buffer[6]=='p'){
                // query_profile

            } else if(buffer[6]=='o'){
                // query_order
            } else{
                if(buffer[7]=='i'){
                    // query_ticket
                } else if(buffer[9]=='i'){
                    // query_train
                } else{
                    // query_transfer
                }
            }
        } else if(buffer[0]=='a'){
            if(buffer[4]=='u'){
                // add_user
            } else{
                // add_train
            }
        } else if(buffer[0]=='l'){
            if(buffer[3]=='i'){
                // login
            } else{
                // logout
            }
        } else if(buffer[0]=='r'){
            if(buffer[2]=='l'){
                // release_train
            } else{
                // refund_ticket
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
                case 'b':
                    // buy_ticket
                    break;
                case 'c':
                    // clean
                    // todo need reconstructing managers !!!
                    cleanAll();
                    init();
                    cout<<"0"<<endl;
                    break;
                case 'd':
                    // delete_train
                    break;
            }
        }
    }

    return 0;
}
