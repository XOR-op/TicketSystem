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
    trainlist.push_back(ptr->trainID);
    for(int i=0;i<ptr->stationNum;i++){
        if(stationlist.find(station_t(ptr->stations[i]))==stationlist.end())stationlist[station_t(ptr->stations[i])]=++stationnum;
        //std::cout<<i<<' '<<stationlist[station_t(ptr->stations[1])]<<endl;
        stationTotrain.insert(stationlist[station_t(ptr->stations[i])]*10000+trainnum,i);
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
    std::vector<std::pair<long long,int>>S=stationTotrain.range(stationlist[station_t(Sstation)]*10000LL,stationlist[station_t(Sstation)]*10000LL+9999);
    std::vector<std::pair<long long,int>>T=stationTotrain.range(stationlist[station_t(Tstation)]*10000LL,stationlist[station_t(Tstation)]*10000LL+9999);
    pse_std::vector<std::pair<int,std::pair<int,int>>>Ans;
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
                    int key;
                    if(order==0){
                        key=calctime(ptr->stopoverTimes[S[i].second],ptr->travelTimes[T[i].second]);//time = dddhhmm
                    }else{
                        key=ptr->prices[T[i].second]-ptr->prices[S[i].second];
                    }
                    std::pair<int,std::pair<int,int>>Element;
                    Element.first=(int)(S[i].first%10000);
                    Element.second.first=S[i].second*100+T[i].second;
                    Element.second.second=key;
                    ansnum++;
                    Ans.push_back(Element);
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
        int p=s[i];
        trainID_t tra=trainlist[Ans[p].first];
        DiskLoc_T loc=trainidToOffset.search(tra).first;
        auto* ptr=cache.get(loc);
        int s=Ans[p].second.first/100,t=Ans[p].second.first%100;
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
        printtime(ptr->travelTimes[t]%10000);
        defaultOut<<' '<<(ptr->prices[t]-ptr->prices[s])<<' ';
        int seat=ptr->seatNum;
        for(int j=s;j<t;j++)seat=std::min(seat,ptr->stationTicketRemains[calcdays(start,startday)][j]);
        defaultOut<<seat<<endl;
    }
}
void TrainManager::Query_transfer(char *Sstation,char *Tstation,int date,int order)
{
    std::vector<std::pair<long long,int>>S=stationTotrain.range(stationlist[station_t(Sstation)]*10000LL,stationlist[station_t(Sstation)]*10000LL+9999);
    std::vector<std::pair<long long,int>>T=stationTotrain.range(stationlist[station_t(Tstation)]*10000LL,stationlist[station_t(Tstation)]*10000LL+9999);
    int minkey=2147483647;
    std::pair<int,std::pair<int,int>>A,B;
    for(int i=0;i<S.size();i++)
        for(int j=0;j<T.size();j++)if(S[i].first%10000!=T[j].first%10000){
            //printf("%d %d %lld %lld\n",i,j,S[i].first,T[j].first);
            trainID_t t1=trainlist[S[i].first%10000],t2=trainlist[T[j].first%10000];
            DiskLoc_T loc=trainidToOffset.search(t1).first,loc2=trainidToOffset.search(t2).first;
            auto* ptr=cache.get(loc);
            auto* ptr2=cache.get(loc2);
            int days=ptr->stopoverTimes[S[i].second]/10000;
            int startday=calcstartday(date,days);
            int start=(ptr->saleDate)/10000;
            int end=(ptr->saleDate)%10000;
            int start2=(ptr2->saleDate)/10000;
            int end2=(ptr2->saleDate)%10000;
            //s_train date is ok?
            if(startday<start||startday>end)continue;
            //check_transfer
            int station1=S[i].second,station2=T[j].second;
            for(int k=station1+1;k<ptr->stationNum;k++){
                for(int h=0;h<station2;h++)if(strcmp(ptr->stations[k],ptr2->stations[h])==0){
                    //printf("%d %d\n",k,h);
                    //T_arrive <= T_leave
                    int stopoverday=0;
                    if(ptr->travelTimes[k]%10000>ptr2->stopoverTimes[h]%10000)stopoverday=1;
                    //t_train date is ok?
                    int hdate=startday,tmp=0;
                    addtime(hdate,tmp,24*60*(ptr->travelTimes[k]/10000+stopoverday));
                    int days2=ptr2->stopoverTimes[h]/10000;
                    int beststartday2=calcstartday(hdate,days2);
                    if(beststartday2>end2)break;
                    int key;
                    if(beststartday2>=start2){
                        if(order==0){
                            key=calctime(ptr->stopoverTimes[station1],ptr->travelTimes[k])+
                                    calctime(ptr->travelTimes[k]%10000,10000*stopoverday+ptr2->stopoverTimes[h])+
                                    calctime(ptr2->stopoverTimes[h],ptr2->travelTimes[station2]);
                        }else{
                            key=ptr->prices[k]-ptr->prices[station1]+ptr2->prices[station2]-ptr2->prices[h];
                        }
                        if(key<minkey){
                            minkey=key;
                            A=std::make_pair((int)(S[i].first%10000),std::make_pair(station1*100+k,date));
                            B=std::make_pair((int)(T[j].first%10000),std::make_pair(h*100+station2,hdate));
                        }
                    }else{
                        hdate=start2;int tmp=0;
                        addtime(hdate,tmp,24*60*(int)(ptr2->stopoverTimes[h]/10000));
                        if(order==0){
                            key=calctime(ptr->stopoverTimes[station1],ptr->travelTimes[k])+
                                calctime(ptr->travelTimes[k]%10000,10000*(start2-beststartday2+stopoverday)+ptr2->stopoverTimes[h])+
                                calctime(ptr2->stopoverTimes[h],ptr2->travelTimes[station2]);
                        }else{
                            key=ptr->prices[k]-ptr->prices[station1]+ptr2->prices[station2]-ptr2->prices[h];
                        }
                        if(key<minkey){
                            minkey=key;
                            A=std::make_pair((int)(S[i].first%10000),std::make_pair(station1*100+k,date));
                            B=std::make_pair((int)(T[j].first%10000),std::make_pair(h*100+station2,hdate));
                        }
                    }
                    break;
                }
            }
        }
    //defaultOut
    if(minkey==2147483647)defaultOut<<0<<endl;
    else{
        //A
        trainID_t tra=trainlist[A.first];
        DiskLoc_T loc=trainidToOffset.search(tra).first;
        auto* ptr=cache.get(loc);
        int s=A.second.first/100,t=A.second.first%100;
        int days=ptr->stopoverTimes[s]/10000;
        int startday=calcstartday(date,days);
        int start=(ptr->saleDate)/10000;
        int end=(ptr->saleDate)%10000;
        defaultOut<<(ptr->trainID.ID)<<' '<<Sstation<<' ';
        printdate(date);
        defaultOut<<' ';
        printtime(ptr->stopoverTimes[s]%10000);
        defaultOut<<" -> "<<ptr->stations[t]<<' ';
        int tdate=date,tmp=0;
        addtime(tdate,tmp,24*60*(int)(ptr->travelTimes[t]/10000-ptr->stopoverTimes[s]/10000));
        printdate(tdate);
        defaultOut<<' ';
        printtime(ptr->travelTimes[t]%10000);
        defaultOut<<' '<<(ptr->prices[t]-ptr->prices[s])<<' ';
        int seat=ptr->seatNum;
        for(int j=s;j<t;j++)seat=std::min(seat,ptr->stationTicketRemains[calcdays(start,startday)][j]);
        defaultOut<<seat<<endl;
        //B
        trainID_t tra2=trainlist[B.first];
        DiskLoc_T loc2=trainidToOffset.search(tra2).first;
        auto* ptr2=cache.get(loc2);
        int s2=B.second.first/100,t2=B.second.first%100;
        days=ptr->stopoverTimes[s2]/10000;
        startday=calcstartday(B.second.second,days);
        start=(ptr2->saleDate)/10000;
        end=(ptr2->saleDate)%10000;
        defaultOut<<(ptr2->trainID.ID)<<' '<<ptr2->stations[s2]<<' ';
        printdate(B.second.second);
        defaultOut<<' ';
        printtime(ptr2->stopoverTimes[s2]%10000);
        defaultOut<<" -> "<<Tstation<<' ';
        tdate=B.second.second,tmp=0;
        addtime(tdate,tmp,24*60*(ptr2->travelTimes[t2]/10000-ptr2->stopoverTimes[s2]/10000));
        printdate(tdate);
        defaultOut<<' ';
        printtime(ptr2->travelTimes[t2]%10000);
        defaultOut<<' '<<(ptr2->prices[t2]-ptr2->prices[s2])<<' ';
        seat=ptr2->seatNum;
        for(int j=s2;j<t2;j++)seat=std::min(seat,ptr2->stationTicketRemains[calcdays(start,startday)][j]);
        defaultOut<<seat<<endl;
    }
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
