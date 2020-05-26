#ifndef TICKETSYSTEM_TRAINMANAGER_H
#define TICKETSYSTEM_TRAINMANAGER_H

#include <fstream>
#include <iostream>
#include <algorithm>
#include "structure.h"
#include "OrderManager.h"
#include "UserManager.h"
#include "../basic_component/cache.h"
#include "../basic_component/LRUBPtree.h"
#include "../include/vector.hpp"
using std::endl;
namespace t_sys {
    class TrainManager{
    private:
        std::fstream trainFile,ticketFile;
        DiskLoc_T file_size,file_size2;
        std::ostream& defaultOut;
        cache::LRUCache<DiskLoc_T,train> train_cache;
        cache::LRUCache<DiskLoc_T ,pending_order> pending_cache;
        bptree::LRUBPTree<trainID_t,DiskLoc_T> trainidToOffset;
        bptree::LRUBPTree<long long,int> stationTotrain; //long long = stationnum+trainnum, int = k-th station
        int trainnum;//0-base
        int stationnum;//1-base
        int ticketnum;//1-base
        pse_std::vector<trainID_t>trainlist;

        static unsigned int myhash( const void * key, int len, unsigned int seed );
        struct stationhash{
            size_t operator()(const station_t ctx) const
            {
                return myhash(ctx.st,strlen(ctx.st),static_cast<size_t>(0xc70f6907UL));
            }
        };
        ds::unordered_map<station_t ,int ,stationhash>stationlist;

        struct freenode{
            freenode* nxt;
            DiskLoc_T pos;
        };
        freenode* head;

        static void loadTrain(std::fstream& ifs,DiskLoc_T offset,train* tra){
            ifs.seekg(offset);
            ifs.read((char*)tra, sizeof(train));
        }
        static void saveTrain(std::fstream& ofs,DiskLoc_T offset,const train* tra){
            ofs.seekp(offset);
            ofs.write((char*)tra, sizeof(train));
        }

        static void loadpendingorder(std::fstream& ifs,DiskLoc_T offset,pending_order* po){
            ifs.seekg(offset);
            ifs.read((char*)po, sizeof(pending_order));
        }
        static void savependingorder(std::fstream& ofs,DiskLoc_T offset,const pending_order* po){
            ofs.seekp(offset);
            ofs.write((char*)po, sizeof(pending_order));
        }


        DiskLoc_T increaseFile(train* tra);
        void transfer_sub_print(const std::pair<int, std::pair<int, int>>& A,int date,const char* station);

    public:
        enum {TIME=0,COST=1};
        int findtrainID(const trainID_t& t);
        void print(int x);
        void printdate(int x);
        void printtime(int x);

        int getDate(const int x){
            return x/10000;
        }

        int getTime(const int x){
            return x%10000;
        }

        static int parsingDate(const char* str){
            // str guaranteed be 5+1 long
            return (str[0]-'0')*1000+(str[1]-'0')*100+(str[3]-'0')*10+str[4]-'0';
        }

        static int parsingTime(const char* str){
            return (str[0]-'0')*1000+(str[1]-'0')*100+(str[3]-'0')*10+str[4]-'0';
        }

        int calcdays(int start,int end);

        int calctime(int start,int end);

        int calcstartday(int date,int days);

        void add_pendingorder(pending_order* record, train* tra);

        void allocate_tickets(OrderManager* ord_manager,train* ptr,const order* Order);

        void addtime(int &date,int &tim,int t);

        bool Add_train(const trainID_t& t, int stationNUM, int seatNUM, char** stations,
                       const int* prices, int startTime, const int* travelTimes, const int* stopoverTimes, int saleDate, char type);//saleDate = mmddmmdd

        bool Delete_train(const trainID_t& t);

        bool Release_train(const trainID_t& t);

        bool Query_train(const trainID_t& t,int date);//date = mmdd

        bool Query_ticket(const char* Sstation,const char* Tstation,int date,int order=TIME);//order: time = 0, cost = 1

        bool Query_transfer(const char *Sstation,const char *Tstation,int date,int order=TIME);

        bool Buy_ticket(UserManager* usr_manager,OrderManager* ord_manager,username_t usr,trainID_t tra,
                        int date,int num,char *Sstation,char *Tstation,bool wait=0);

        bool Refund_ticket(UserManager* usr_manager,OrderManager* ord_manager,username_t usr,int x=1);

        TrainManager(const std::string& file_path, const std::string& pending_path, const std::string& trainid_index_path, const std::string& station_index_path);

        ~TrainManager();

        static void Init(const std::string& file_path,const std::string& pending_path);
    };

}
#endif //TICKETSYSTEM_TRAINMANAGER_H