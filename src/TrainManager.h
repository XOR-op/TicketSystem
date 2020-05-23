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
        std::fstream trainFile;
        DiskLoc_T file_size;
        std::ostream& defaultOut;
        cache::LRUCache<DiskLoc_T,train> cache;
        bptree::LRUBPTree<trainID_t,DiskLoc_T> trainidToOffset;
        bptree::LRUBPTree<long long,int> stationTotrain; //long long = stationnum+trainnum, int = k-th station
        int trainnum;//0-base
        int stationnum;//1-base
        pse_std::vector<trainID_t>trainlist;

        static inline unsigned int myhash( const void * key, int len, unsigned int seed )
        {
            const unsigned int m = 0x5bd1e995;
            const int r = 24;
            unsigned int h = seed ^ len;
            const unsigned char * data = (const unsigned char *)key;
            while(len >= 4){
                unsigned int k = *(unsigned int *)data;
                k *= m;
                k ^= k >> r;
                k *= m;
                h *= m;
                h ^= k;
                data += 4;
                len -= 4;
            }
            switch(len){
                case 3: h ^= data[2] << 16;
                case 2: h ^= data[1] << 8;
                case 1: h ^= data[0];
                    h *= m;
            };
            h ^= h >> 13;
            h *= m;
            h ^= h >> 15;
            return h;
        }
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

        DiskLoc_T increaseFile(train* tra);

    public:
        enum {TIME=0,COST=1};
        int findtrainID(const trainID_t& t){
            auto rt=trainidToOffset.search(t);
            if(rt.second)return 1;
            else return 0;
        }
        void print(int x){
            if(x<10)defaultOut<<'0'<<x;
            else defaultOut<<x;
        }
        void printdate(int x){
            print(x/100);
            defaultOut<<'-';
            print(x%100);
        }
        void printtime(int x){
            print(x/100);
            defaultOut<<':';
            print(x%100);
        }

        static int parsingDate(const char* str){
            // str guaranteed be 5+1 long
            return (str[0]-'0')*1000+(str[1]-'0')*100+(str[3]-'0')*10+str[4]-'0';
        }

        int calcdays(int start,int end){
            if(start/100==end/100)return end-start;
            else {
                int ans=0;
                if(start/100==6)ans=30-start%100;
                else ans=31-start%100;
                for(int i=start/100+1;i<end/100;i++){
                    if(i==7||i==8||i==10)ans+=31;
                        else ans+=30;
                }
                return ans+end%100;
            }
        }

        int calctime(int start,int end){
            int mm=end%100-start%100;
            start/=100,end/=100;
            if(mm<0)mm+=60,end--;
            int hh=end/100-start/100;
            start/=100,end/=100;
            if(hh<0)hh+=24,end--;
            int ddd=end-start;
            return ddd*10000+hh*100+mm;
        }

        int calcstartday(int date,int days){
            if(date%100>days)return date-days;
            days-=date%100;
            if(date/100==9)return calcstartday(831,days);
            if(date/100==8)return calcstartday(731,days);
            if(date/100==7)return calcstartday(630,days);
            if(date/100==6)return calcstartday(531,days);
            return -1;
        }

        void addtime(int &date,int &tim,int t);

        bool Add_train(const trainID_t& t, int stationNUM, int seatNUM, char** stations,
                       const int* prices, int startTime, const int* travelTimes, const int* stopoverTimes, int saleDate, char type);//saleDate = mmddmmdd

        bool Delete_train(const trainID_t& t);

        bool Release_train(const trainID_t& t);

        bool Query_train(const trainID_t& t,int date);//date = mmdd

        bool Query_ticket(char* Sstation,char* Tstation,int date,int order=TIME);//order: time = 0, cost = 1

        bool Query_transfer(char *Sstation,char *Tstation,int date,int order=TIME);

        bool Buy_ticket(UserManager* usr_manager,OrderManager* ord_manager,username_t usr,trainID_t tra,
                        int date,int num,char *Sstation,char *Tstation,bool wait=0);

        bool Refund_ticket(UserManager* usr_manager,OrderManager* ord_manager,username_t usr,int x=1);

        TrainManager(const std::string& file_path,const std::string& trainid_index_path,const std::string& station_index_path);

        ~TrainManager();

        static void Init(const std::string& path);
    };

}
#endif //TICKETSYSTEM_TRAINMANAGER_H