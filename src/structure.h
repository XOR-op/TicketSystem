//
// Created by vortox on 26/3/20.
//

#ifndef TICKETSYSTEM_STRUCTURE_H
#define TICKETSYSTEM_STRUCTURE_H

#include <cstdint>
#include <cstring>
namespace t_sys{
    typedef uint64_t DiskLoc_T;
    struct username_t{
        char name[21];
        bool operator<(const username_t& rhs)const { return strcmp(name,rhs.name)<0;}
        bool operator==(const username_t& rhs)const {return !strcmp(name,rhs.name);}
    };
    struct user{
        username_t username;
        char password[31];
        char name[21];
        char mailAddr[31];
        int privilege;
        DiskLoc_T orderOffset;
    };
    struct trainID_t{
        char ID[21];
        trainID_t(){
            ID[0]='\0';
        }
        trainID_t(const char* id){
            strcpy(ID,id);
        }
        bool operator<(const trainID_t& rhs)const { return strcmp(ID,rhs.ID)<0;}
        bool operator==(const trainID_t& rhs)const {return !strcmp(ID,rhs.ID);}
        bool operator!=(const trainID_t& rhs)const {return strcmp(ID,rhs.ID)!=0;}
    };
    struct train{
        DiskLoc_T offset;
        trainID_t trainID;
        int stationNum;     // n
        int seatNum;
        int startTime;
        int saleDate; //mmddmmdd
        bool releaseState;
        char type;
        //char** stations;    // n
        //int* prices;        // n-1
        //int* travelTimes;   // n-1
        //int* stopoverTimes; // n-2
        //int* stationTicketRemains;  // n-1
        char stations[101][21];
        int prices[101];
        int travelTimes[101];
        int stopoverTimes[101];
        int stationTicketRemains[101];
        /*void release(){
            delete prices;
            delete travelTimes;
            delete stopoverTimes;
            delete stationTicketRemains;
            for(int i=0;i<stationNum;++i)
                delete stations[i];
            delete stations;
        }*/
    };//because wtl is naive, so the arrays are fixed length.

    struct order{
        enum STATUS{SUCCESS, PENDING, REFUNDED};
        STATUS stat;
        int leaveTime;
        int arriveTime;
        int price;
        int num;
        char trainID[21];
        char from[41];
        char to[41];
    };

}
namespace std{
    template <>
    struct hash<t_sys::username_t>{
        std::size_t operator()(const t_sys::username_t& key)const {
            std::size_t hash=0;
            for(auto c:key.name){
                if(!c)break;
                hash=(hash*239+c)%1000007;
            }
            return hash;
        }
    };
}
#endif //TICKETSYSTEM_STRUCTURE_H
