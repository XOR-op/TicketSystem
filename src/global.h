//
// Created by vortox on 28/4/20.
//

#ifndef TICKETSYSTEM_GLOBAL_H
#define TICKETSYSTEM_GLOBAL_H
#include <memory>
#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>
#include "../include/debug.h"
namespace t_sys{
    typedef uint32_t DiskLoc_T;
    const static DiskLoc_T DISKLOC_MAX=UINT32_MAX;
    typedef const char* const str_t ;
#ifndef NDEBUG
    str_t USER_PATH="/tmp/ticket_system/user_manager.db";
    str_t TRAIN_PATH="/tmp/ticket_system/train_manager.db";
    str_t ORDER_PATH="/tmp/ticket_system/order_manager.db";
    str_t USER_INDEX_PATH="/tmp/ticket_system/user_manager.idx";
    str_t TRAIN_TRAIN_ID_INDEX_PATH="/tmp/ticket_system/train_id.idx";
    str_t TRAIN_STATION_INDEX_PATH="/tmp/ticket_system/train_station.idx";
    str_t PENDING_PATH="/tmp/ticket_system/pending.db";
    str_t TRAIN_INFO_PATH="/tmp/ticket_system/ticket.dat";
    str_t STATION_INFO_PATH="/tmp/ticket_system/station.dat";
#else
    str_t USER_PATH="./user_manager.db";
    str_t TRAIN_PATH="./train_manager.db";
    str_t ORDER_PATH="./order_manager.db";
    str_t USER_INDEX_PATH="./user_manager.idx";
    str_t TRAIN_TRAIN_ID_INDEX_PATH="./train_id.idx";
    str_t TRAIN_STATION_INDEX_PATH="./train_station.idx";
    str_t PENDING_PATH="./pending.db";
    str_t TRAIN_INFO_PATH="./ticket.dat";
    str_t STATION_INFO_PATH="./station.dat";
    str_t OFFSET_INFO_PATH="./offset.dat";
#endif
    const int USER_NAME_LEN=20,
            PASSWORD_LEN=30,
            NAME_LEN=5,
            MAIL_ADDR_LEN=30,
            TRAIN_ID_LEN=20,
            STATION_NUM=101,
            STATIONS_LEN=10;
    constexpr int l_str(int x){return x+1;}
    constexpr int l_han(int x){return 4*x+1;}
    static void init_subprocess(const std::string& path) {
        std::fstream f(path, std::ios::out | std::ios::binary);
        char buf[sizeof(DiskLoc_T)];
        char* ptr = buf;
        DiskLoc_T sz = sizeof(DiskLoc_T);
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
        write_attribute(sz);
#undef write_attribute
        f.write(buf, sizeof(buf));
        f.close();
    }

}
#endif //TICKETSYSTEM_GLOBAL_H
