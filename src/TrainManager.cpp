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

void TrainManager::Init(const std::string& path){
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
               const int* prices, int startTime, const int* travelTimes, const int* stopoverTimes, int saleDate, char type){//saleDate = mddmdd
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
    for(int i=1;i<stationNUM;i++)tra.prices[i]=prices[i-1];
    for(int i=0;i<stationNUM-1;i++)tra.travelTimes[i]=travelTimes[i];
    for(int i=1;i<stationNUM-1;i++)tra.stopoverTimes[i]=stopoverTimes[i-1];
    int tmp=0;
    for(int day=saleDate/10000;day<=saleDate%10000;addtime(day,tmp,24*60))
        for(int i=0;i<stationNUM-1;i++)tra.stationTicketRemains[calcdays(saleDate/10000,day)][i]=seatNUM;
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
    }
    ptr->releaseState=true;
    trainlist[trainnum]=ptr->trainID;
    for(int i=0;i<ptr->stationNum;i++){
        if(stationlist[ptr->stations[i]]==0)stationlist[ptr->stations[i]]=++stationnum;
        //std::cout<<stationlist["B"]<<endl;
        stationTotrain.insert(stationlist[ptr->stations[i]]*10000+trainnum,i);
    }
    trainnum++;
    int date=0,tim=ptr->startTime;
    for(int i=0;i<ptr->stationNum;i++){
        int tmp=ptr->travelTimes[i];
        if(i==0)ptr->travelTimes[i]=0;
        else ptr->travelTimes[i]=date*10000+tim;
        if(i!=0&&i!=(ptr->stationNum)-1)addtime(date,tim,ptr->stopoverTimes[i]);
        if(i==(ptr->stationNum)-1)ptr->stopoverTimes[i]=0;
        else ptr->stopoverTimes[i]=date*10000+tim;
        addtime(date,tim,tmp);
        if(i>0)ptr->prices[i]+=ptr->prices[i-1];
    }
    //maybe more things need to do
    cache.dirty_bit_set(loc);
    defaultOut<<"0"<<endl;
    return true;
}
bool TrainManager::Query_train(const trainID_t& t,int date){//date = mdd
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
    int tmp=0;
    int kday=calcdays(start,date);
    for(int i=0;i<ptr->stationNum;i++){
        defaultOut<<(ptr->stations[i])<<' ';
        if(i==0)defaultOut<<"xx-xx xx:xx";
        else printdate(date),defaultOut<<' ',printtime(ptr->travelTimes[i]%10000);
        defaultOut<<" -> ";
        if(i!=0&&i!=(ptr->stationNum)-1)addtime(date,tmp,24*60*(ptr->stopoverTimes[i]/10000-ptr->travelTimes[i]/10000));
        if(i==(ptr->stationNum)-1)defaultOut<<"xx-xx xx:xx";
        else printdate(date),defaultOut<<' ',printtime(ptr->stopoverTimes[i]%10000);
        addtime(date,tmp,24*60*(ptr->travelTimes[i+1]/10000-ptr->stopoverTimes[i]/10000));
        if(i==0)defaultOut<<" 0";
        else defaultOut<<' '<<(ptr->prices[i])-(ptr->prices[i-1]);
        if(i==(ptr->stationNum)-1)defaultOut<<" x"<<endl;
        else defaultOut<<' '<<(ptr->stationTicketRemains[kday][i])<<endl;
    }
    return true;
}
void TrainManager::Query_ticket(char* Sstation,char* Tstation,int date,int order)
{
    std::cout<<stationlist["B"]<<endl;
    std::vector<std::pair<long long,int>>S=stationTotrain.range(stationlist[Sstation]*10000LL,stationlist[Sstation]*10000LL+9999);
    std::vector<std::pair<long long,int>>T=stationTotrain.range(stationlist[Tstation]*10000LL,stationlist[Tstation]*10000LL+9999);
    pse_std::vector<std::pair<DiskLoc_T ,std::pair<int,int>>>Ans;
    int ansnum=0;
    for(int i=0,j=0;i<S.size()&&j<T.size();){
        if(S[i].first%10000<T[j].first%10000)i++;
        else if(S[i].first%10000>T[j].first%10000)j++;
        else{
            if(S[i].second<T[j].second){
                trainID_t t=trainlist[S[i].first%10000];
                DiskLoc_T loc=trainidToOffset.search(t).first;
                auto* ptr=cache.get(loc);
                int days=ptr->stopoverTimes[S[i].second]/10000;
                int startday=calcstartday(date,days);
                int start=(ptr->saleDate)/10000;
                int end=(ptr->saleDate)%10000;
                if(startday>=start&&startday<=end){
                    int key=0;
                    if(order==0){
                        key=ptr->travelTimes[T[i].second]-ptr->stopoverTimes[S[i].second];//time = dddmmss
                    }else{
                        key=ptr->prices[T[i].second]-ptr->prices[S[i].second];
                    }
                    Ans[ansnum++]=std::make_pair(loc,std::make_pair(S[i].second*100+T[i].second,key));
                }
            }
            i++,j++;
        }
    }
    defaultOut<<ansnum<<endl;
    static const int maxN=10000;
    static int s[maxN];
    for(int i=0;i<ansnum;i++)s[i]=i;
    auto cmp=[&Ans](int x,int y)-> bool {return Ans[x].second.second<Ans[y].second.second;};
    std::sort(s,s+ansnum,cmp);
    //defaultOut
    for(int i=0;i<ansnum;i++){
        auto* ptr=cache.get(Ans[i].first);
        int s=Ans[i].second.first/100,t=Ans[i].second.second%100;
        int days=ptr->stopoverTimes[s]/10000;
        int startday=calcstartday(date,days);
        int start=(ptr->saleDate)/10000;
        int end=(ptr->saleDate)%10000;
        defaultOut<<(ptr->trainID.ID)<<' '<<Sstation<<' ';
        printdate(date);
        defaultOut<<' ';
        printtime(ptr->stopoverTimes[s]%10000);
        defaultOut<<" -> "<<Tstation<<' ';
        int tdate=date,tmp=0;
        addtime(tdate,tmp,24*60*(ptr->travelTimes[t]/10000-ptr->stopoverTimes[s]/10000));
        printdate(tdate);
        defaultOut<<' ';
        printtime(ptr->travelTimes[s]%10000);
        defaultOut<<(ptr->prices[t]-ptr->prices[s])<<' ';
        int seat=ptr->seatNum;
        for(int j=s;j<t;j++)seat=std::min(seat,ptr->stationTicketRemains[calcdays(start,startday)][j]);
        defaultOut<<seat<<endl;
    }
}
void TrainManager::Query_transfer(char *Sstation,char *Tstation,int date,int order)
{
    //pse_std::vector<std::pair<long long,int>>S=stationTotrain.range(stationlist[Sstation]*10000LL,stationlist[Sstation]*10000LL+9999);
    //pse_std::vector<std::pair<long long,int>>T=stationTotrain.range(stationlist[Tstation]*10000LL,stationlist[Tstation]*10000LL+9999);
    /*for(int i=0;i<S.size();i++)
        for(int j=0;j<T.size();j++)if(S[i].first%10000!=T[i].first%10000){
            //check_transfer
            if(check==true){

            }
        }*/
}
TrainManager::TrainManager(const std::string& file_path,const std::string& trainid_index_path,const std::string& station_index_path)
        :cache(51,[this](DiskLoc_T off,train* tra){loadTrain(trainFile,off,tra);},
               [this](DiskLoc_T off,const train* tra){saveTrain(trainFile,off,tra);}),
         trainidToOffset(trainid_index_path,107),
         stationTotrain(station_index_path,157),
         head(NULL),defaultOut(std::cout),trainnum(0),stationnum(0)
{
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
