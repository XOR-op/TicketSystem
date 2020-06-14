#ifndef TICKETSYSTEM_TRAINMANAGER_H
#define TICKETSYSTEM_TRAINMANAGER_H

#include <fstream>
#include <iostream>
#include <algorithm>
#include "structure.h"
#include "UserOrderManager.h"
#include "UserManager.h"
#include "PendingTicketManager.h"
#include "../basic_component/SLRU.h"
#include "../basic_component/LRUBPtree.h"
#include "../basic_component/SLRUBPtree.h"
#include "../include/vector.hpp"
//using std::endl;
namespace t_sys {
    class TrainManager{
    private:
        std::fstream trainFile;
        DiskLoc_T train_file_size;
        std::ostream& defaultOut;
        cache::SLRUCache<DiskLoc_T,train> train_cache;
        bptree::LRUBPTree<trainID_t,DiskLoc_T> trainidToOffset;
        bptree::SLRUBPTree<long long,int> stationTotrain; //long long = station_num+train_num, int = k-th station
        int train_num;//0-base
        int station_num;//1-base
        int ticket_num;//1-base
        ds::vector<trainID_t>trainlist;

        ds::unordered_map<station_t ,int>stationlist; // stationName to stationID

        ds::unordered_map<DiskLoc_T , int>trainOffset;

        std::string train_info_path,station_info_path,offset_info_path;

        void loadTrain(std::fstream& ifs, DiskLoc_T offset, train* tra);
        void saveTrain(std::fstream& ofs, DiskLoc_T offset, train* tra);
        int getsize(train* t);
        DiskLoc_T increaseFile(train* tra);
        void print_ticket(const std::pair<int, std::pair<int, int>>& A,int date);
        int findtrainID(const trainID_t& t);

        std::ostream& print(int x);
        std::ostream& printdate(int x);
        std::ostream& printtime(int x);

    public:
        enum {TIME=0,COST=1};

        bool Add_train(const trainID_t& t, int stationNUM, int seatNUM, char** stations,
                       const int* prices, int startTime, const int* travelTimes, const int* stopoverTimes, int saleDate, char type);//saleDate = mmddmmdd

        bool Delete_train(const trainID_t& t);

        bool Release_train(const trainID_t& t);

        bool Query_train(const trainID_t& t,int date);//date = mmdd

        bool Query_ticket(const char* Sstation,const char* Tstation,int date,int order=TIME);//order: time = 0, cost = 1

        bool Query_transfer(const char *Sstation,const char *Tstation,int date,int order=TIME);

        bool Buy_ticket(UserManager* usr_manager, UserOrderManager* ord_manager,PendingTicketManager* pend_manager, username_t usr, trainID_t tra,
                        int date, int num, char *Sstation, char *Tstation, bool wait=0);

        bool Refund_ticket(UserManager* usr_manager, UserOrderManager* ord_manager,PendingTicketManager* pend_manager, username_t usr, int x=1);

        TrainManager(const std::string& file_path, const std::string& trainid_index_path,
                     const std::string& station_index_path, const std::string& train_info_path,
                     const std::string& station_info_path,const std::string& offset_info_path);

        ~TrainManager();

        static void Init(const std::string& file_path);
    };

}
#endif //TICKETSYSTEM_TRAINMANAGER_H