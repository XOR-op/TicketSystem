#include "TrainManager.h"
using namespace t_sys;
DiskLoc_T TrainManager::increaseFile(train* tra){
    if(head==NULL) {
        DiskLoc_T rt=file_size;
        tra->offset=rt;
        trainFile.seekp(file_size);
        trainFile.write((char *) tra, sizeof(train));
        file_size += sizeof(train);
        return rt;
    }else{
        DiskLoc_T rt=head->pos;
        tra->offset=rt;
        trainFile.seekp(head->pos);
        trainFile.write((char *)tra, sizeof(train));
        head=head->nxt;
        return rt;
    }
}

void TrainManager::create(const std::string& path){
    std::fstream f(path,ios::out|ios::binary);
    char buf[sizeof(DiskLoc_T)];
    char* ptr = buf;
    DiskLoc_T sz=sizeof(DiskLoc_T);
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
    write_attribute(sz);
#undef write_attribute
    f.write(buf,sizeof(buf));
    f.close();
}

void TrainManager::addtime(int &date,int &tim,int t){
    int d=t/(24*60);t%=(24*60);
    date+=d;
    int h=t/60;t%=60;
    int min=t+tim%100;
    tim=(int)(tim/100+min/60+h)*100+min%60;
    if(tim>2359)tim-=2400,date++;
    if(date/100==6&&date%100>30)date=700+date%100-30;
    if(date/100==7&&date%100>31)date=800+date%100-31;
    if(date/100==8&&date%100>31)date=900+date%100-31;
}
bool TrainManager::Add_train(const trainID_t& t, int stationNUM, int seatNUM, char** stations,
               const int* prices, int startTime, const int* travelTimes, const int* stopoverTimes, int saleDate, char type){
    // construct
    if(findtrainID(t)){
        defaultOut<<"-1"<<endl;
        return false;
    }
    train tra{};
    strcpy(tra.trainID.ID,t.ID);
    tra.stationNum=stationNUM;
    tra.seatNum=seatNUM;
    tra.startTime=startTime;
    tra.saleDate=saleDate;
    tra.type=type;
    tra.releaseState=false;
    for(int i=0;i<stationNUM;i++)strcpy(tra.stations[i],stations[i]);
    for(int i=0;i<stationNUM-1;i++)tra.prices[i]=prices[i];
    for(int i=0;i<stationNUM-1;i++)tra.travelTimes[i]=travelTimes[i];
    for(int i=0;i<stationNUM-2;i++)tra.stopoverTimes[i]=stopoverTimes[i];
    for(int i=0;i<stationNUM-1;i++)tra.stationTicketRemains[i]=seatNUM;
    // write back
    DiskLoc_T off=increaseFile(&tra);
    trainidToOffset.insert(t,off);
    defaultOut<<"0"<<endl;
    return true;
}
bool TrainManager::Delete_train(const trainID_t& t){
    if(!findtrainID(t)){
        defaultOut<<"-1"<<endl;
        return false;
    }
    DiskLoc_T loc=trainidToOffset.search(t).first;
    auto* ptr=cache.get(loc);
    if(ptr->releaseState){
        defaultOut<<"-1"<<endl;
        return false;
    }
    //add to freenode
    auto* tmp=new freenode;
    tmp->nxt=head;
    tmp->pos=loc;
    head=tmp;
    //delete
    cache.remove(loc);
    trainidToOffset.remove(t);

    defaultOut<<"0"<<endl;
    return true;
}
bool TrainManager::Release_train(const trainID_t& t){
    if(!findtrainID(t)){
        defaultOut<<"-1"<<endl;
        return false;
    }
    DiskLoc_T loc=trainidToOffset.search(t).first;
    auto* ptr=cache.get(loc);
    if(ptr->releaseState){
        defaultOut<<"-1"<<endl;
        return false;
    }else ptr->releaseState=true;
    cache.dirty_bit_set(loc);

    // more things need to do to prepare for the query
    defaultOut<<"0"<<endl;
    return true;
}
bool TrainManager::Query_train(const trainID_t& t,int date){//date = mmdd
    if(!findtrainID(t)){
        defaultOut<<"-1"<<endl;
        return false;
    }
    DiskLoc_T loc=trainidToOffset.search(t).first;
    auto* ptr=cache.get(loc);
    int start=(ptr->saleDate)/10000;
    int end=(ptr->saleDate)%10000;
    if(!ptr->releaseState||date<start||date>end){
        defaultOut<<"-1"<<endl;
        return false;
    }
    defaultOut<<(ptr->trainID.ID)<<' '<<(ptr->type)<<endl;
    int tim=ptr->startTime;
    for(int i=0;i<ptr->stationNum;i++){
        defaultOut<<(ptr->stations[i])<<' ';
        if(i==0)defaultOut<<"xx-xx xx:xx";
        else printdate(date),defaultOut<<' ',printtime(tim);
        defaultOut<<" -> ";
        if(i!=0&&i!=(ptr->stationNum)-1)addtime(date,tim,ptr->stopoverTimes[i-1]);
        if(i==(ptr->stationNum)-1)defaultOut<<"xx-xx xx:xx";
        else printdate(date),defaultOut<<' ',printtime(tim);
        addtime(date,tim,ptr->travelTimes[i]);
        if(i==0)defaultOut<<" 0";
        else defaultOut<<' '<<(ptr->prices[i-1]);
        if(i==(ptr->stationNum)-1)defaultOut<<" x"<<endl;
        else defaultOut<<' '<<(ptr->stationTicketRemains[i])<<endl;
    }
    return true;
}
TrainManager::TrainManager(const std::string& file_path,const std::string& trainid_index_path,bool create_flag)
        :cache(51,[this](DiskLoc_T off,train* tra){loadTrain(trainFile,off,tra);},
               [this](DiskLoc_T off,const train* tra){saveTrain(trainFile,off,tra);}),
         trainidToOffset(trainid_index_path,107,create_flag),head(NULL),defaultOut(std::cout)
{
    if(create_flag)create(file_path);
    trainFile.open(file_path);
    //metadata
    char buf[sizeof(file_size)];
    char* ptr = buf;
    trainFile.seekg(0);
    trainFile.read(buf, sizeof(buf));
#define read_attribute(ATTR) memcpy((void*)&ATTR,ptr,sizeof(ATTR));ptr+=sizeof(ATTR)
    read_attribute(file_size);
#undef read_attribute
}

TrainManager::~TrainManager(){
    // write metadata
    char buf[sizeof(file_size)];
    char* ptr = buf;
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
    write_attribute(file_size);
#undef write_attribute
    trainFile.seekg(0);
    trainFile.write(buf, sizeof(buf));
    cache.destruct();
    trainFile.close();
}
