#ifndef TICKETSYSTEM_TRAINMANAGER_H
#define TICKETSYSTEM_TRAINMANAGER_H

#include <fstream>
#include <iostream>
#include "structure.h"
#include "OrderManager.h"
#include "../basic_component/cache.h"
#include "../basic_component/LRUBPtree.h"
using std::endl;
namespace t_sys {
    class TrainManager{
    private:
        std::fstream trainFile;
        DiskLoc_T file_size;
        std::ostream& defaultOut;
        cache::LRUCache<DiskLoc_T,train> cache;
        bptree::LRUBPTree<trainID_t,DiskLoc_T> trainidToOffset;

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

        void create(const std::string& path);
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

        void addtime(int &date,int &tim,int t);

        bool Add_train(const trainID_t& t, int stationNUM, int seatNUM, char** stations,
                       const int* prices, int startTime, const int* travelTimes, const int* stopoverTimes, int saleDate, char type);

        bool Delete_train(const trainID_t& t);

        bool Release_train(const trainID_t& t);

        bool Query_train(const trainID_t& t,int date);//date = mmdd

        TrainManager(const std::string& file_path,const std::string& trainid_index_path,bool create_flag=false);

        ~TrainManager();
    };

}
#endif //TICKETSYSTEM_TRAINMANAGER_H