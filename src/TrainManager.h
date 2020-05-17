#ifndef TICKETSYSTEM_TRAINMANAGER_H
#define TICKETSYSTEM_TRAINMANAGER_H

#include <fstream>
#include <iostream>
#include <algorithm>
#include "structure.h"
#include "OrderManager.h"
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
        ds::unordered_map<int ,trainID_t>trainlist;

        struct ptrcmp{
            bool operator()(const char *s1,const char *s2) const{
                return strcmp(s1,s2)<0;
            }
        };
        ds::unordered_map<char* ,int ,ptrcmp>stationlist;

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

        void Query_ticket(char* Sstation,char* Tstation,int date,int order=0);//order: time = 0, cost = 1

        void Query_transfer(char *Sstation,char *Tstation,int date,int order=0);

        TrainManager(const std::string& file_path,const std::string& trainid_index_path,const std::string& station_index_path);

        ~TrainManager();

        static void Init(const std::string& path);
    };

}
#endif //TICKETSYSTEM_TRAINMANAGER_H