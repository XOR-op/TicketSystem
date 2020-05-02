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
