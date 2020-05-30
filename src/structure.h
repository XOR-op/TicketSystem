//
// Created by vortox on 26/3/20.
//

#ifndef TICKETSYSTEM_STRUCTURE_H
#define TICKETSYSTEM_STRUCTURE_H

#include <cstdint>
#include <cstring>
#include "global.h"
#include "../include/unordered_map.h"

namespace t_sys {
    typedef uint64_t DiskLoc_T;

    struct username_t {
        char name[l_str(USER_NAME_LEN)];
        bool operator<(const username_t& rhs) const { return strcmp(name, rhs.name) < 0; }
        bool operator==(const username_t& rhs) const { return !strcmp(name, rhs.name); }
    };

    struct user {
        username_t username;
        char password[l_str(PASSWORD_LEN)];
        char name[l_han(NAME_LEN)];
        char mailAddr[l_str(MAIL_ADDR_LEN)];
        int privilege;
        DiskLoc_T orderOffset;
        size_t orderSize;
    };

    struct trainID_t {
        char ID[l_str(TRAIN_ID_LEN)];
        trainID_t() {
            ID[0] = '\0';
        }
        explicit trainID_t(const char* id) {
            strcpy(ID, id);
        }
        bool operator<(const trainID_t& rhs) const { return strcmp(ID, rhs.ID) < 0; }
        bool operator==(const trainID_t& rhs) const { return !strcmp(ID, rhs.ID); }
        bool operator!=(const trainID_t& rhs) const { return strcmp(ID, rhs.ID) != 0; }
    };

    struct station_t {
        char st[l_han(STATIONS_LEN)];
        station_t() {
            st[0] = '\0';
        }
        explicit station_t(const char* s) {
            strcpy(st, s);
        }
        bool operator<(const station_t& rhs) const { return strcmp(st, rhs.st) < 0; }
        bool operator==(const station_t& rhs) const { return !strcmp(st, rhs.st); }
        bool operator!=(const station_t& rhs) const { return strcmp(st, rhs.st) != 0; }
    };

    struct train {
        DiskLoc_T offset;
        trainID_t trainID;
        int stationNum;     // n
        int seatNum;
        int startTime;
        int saleDate; //mmddmmdd
        bool releaseState;
        char type;
        char stations[STATION_NUM][l_han(STATIONS_LEN)];
        int prices[STATION_NUM];
        int travelTimes[STATION_NUM];
        int stopoverTimes[STATION_NUM];
        DiskLoc_T ticket_head,ticket_end;
        //release时，travelTimes[]、stopoverTimes[] 将会做一个前缀和，也就是变成每个站的离开时间和到达时间
        //形式为dddmmss,ddd是天没有月的概念，始发站为0
        int stationTicketRemains[101][101];
    };

    struct order {
        enum STATUS {
            SUCCESS, PENDING, REFUNDED
        };
        static const int NONE_TIME = 0;
        STATUS stat;
        int leaveTime;  // mmddhhmm e.g. 7311245 means 07-30 12:45
        int arriveTime;
        int price;
        int num;
        int day;
        int key;
        char trainID[21];
        char from[41];
        char to[41];
    };

    struct pending_order{
        DiskLoc_T block,nxt;
        int offset_in_block,day,key,num,s,t;
    };

}
namespace std {
    template<>
    struct hash<t_sys::username_t> {
        std::size_t operator()(const t_sys::username_t& key) const {
            std::size_t hash = 0;
            for (auto c:key.name) {
                if (!c)break;
                hash = (hash*239+c)%1000007;
            }
            return hash;
        }
    };
}
#endif //TICKETSYSTEM_STRUCTURE_H
