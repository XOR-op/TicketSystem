//
// Created by vortox on 21/5/20.
//

#ifndef TICKETSYSTEM_COMMANDS_H
#define TICKETSYSTEM_COMMANDS_H
#include <cstdio>
#include <iostream>
#include "global.h"
#include "TrainManager.h"
#include "UserManager.h"
#include "UserOrderManager.h"
namespace t_sys{

    class vars{
    public:
        TrainManager* train_mgr;
        UserManager* user_mgr;
        UserOrderManager* order_mgr;
        PendingTicketManager* pending_mgr;
        vars(TrainManager* tm, UserManager* um, UserOrderManager* om,PendingTicketManager* pm)
        :train_mgr(tm),user_mgr(um),order_mgr(om),pending_mgr(pm){}
    };

    bool needInit();

    char getOption(std::istream& ifs=std::cin);

}
#endif //TICKETSYSTEM_COMMANDS_H
