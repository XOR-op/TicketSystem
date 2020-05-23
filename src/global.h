//
// Created by vortox on 28/4/20.
//

#ifndef TICKETSYSTEM_GLOBAL_H
#define TICKETSYSTEM_GLOBAL_H
#include <memory>
namespace t_sys{
    typedef std::size_t DiskLoc_T;
    typedef const char* const str_t ;
    str_t USER_PATH="/tmp/ticket_system/user_manager.db";
    str_t TRAIN_PATH="/tmp/ticket_system/train_manager.db";
    str_t ORDER_PATH="/tmp/ticket_system/order_manager.db";
    str_t USER_INDEX_PATH="/tmp/ticket_system/user_manager.idx";
    str_t TRAIN_TRAIN_ID_INDEX_PATH="/tmp/ticket_system/train_id.idx";
    str_t TRIAN_STATION_INDEX_PATH="/tmp/ticket_system/train_station.idx";
}
#endif //TICKETSYSTEM_GLOBAL_H
