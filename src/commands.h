//
// Created by vortox on 21/5/20.
//

#ifndef TICKETSYSTEM_TEST1_H
#define TICKETSYSTEM_TEST1_H
#include "main_helper.h"
namespace t_sys{
    void query_ticket(vars v);

    void buy_ticket(vars v);

    void login(vars v);

    void logout(vars v);

    void modify_profile(vars v);

    void query_profile(vars v);

    void query_order(vars v);

    void add_user(vars v);

    void add_train(vars v);

    void release_train(vars v);

    void query_train(vars v);

    void delete_train(vars v);

    void query_transfer(vars v);

    void refund_ticket(vars v);

}
#endif //TICKETSYSTEM_COMMANDS_H
