#include "TrainManager.h"
#include "../include/SerializationHelper.h"

using namespace t_sys;
using std::pair;
int calcstartday(int date, int days) {
    if (date%100 > days)return date-days;
    days -= date%100;
    if (date/100 == 9)return calcstartday(831, days);
    if (date/100 == 8)return calcstartday(731, days);
    if (date/100 == 7)return calcstartday(630, days);
    if (date/100 == 6)return calcstartday(531, days);
    return -1;
}
int min_fmt(int x) {
    // format in dddhhmm
    return x%100+60*(x/100%100)+24*60*(x/10000);
}
int calctime(int start, int end) {
    // return dddhhmm
    int minute = min_fmt(end)-min_fmt(start);
    int rt = minute/(24*60)*10000; // day
    minute %= (24*60);
    rt += minute/60*100+minute%60; // hour and minute
    return rt;
}
int TrainManager::findtrainID(const trainID_t& t) {
    auto rt = trainidToOffset.search(t);
    if (rt.second)return 1;
    else return 0;
}
std::ostream& TrainManager::print(int x) {
    if (x < 10)defaultOut << '0' << x; else defaultOut << x;
    return defaultOut;
}
std::ostream& TrainManager::printdate(int x) {
    print(x/100);
    defaultOut << '-';
    print(x%100);
    return defaultOut;
}
std::ostream& TrainManager::printtime(int x) {
    print(x/100);
    defaultOut << ':';
    print(x%100);
    return defaultOut;
}
int getDate(const int x) {
    return x/10000;
}

int getTime(const int x) {
    return x%10000;
}
int station_id_f(int x) { return x/10000; }
int train_id_f(int x) { return x%10000; }
void TrainManager::loadTrain(std::fstream& ifs, DiskLoc_T offset, train* tra) {
    ifs.seekg(offset);
    char buffer[trainOffset[offset]];
    ifs.read(buffer, sizeof(buffer));
    char* buf = buffer;

#define read_attribute(ATTR) memcpy((void*)&ATTR,buf,sizeof(ATTR));buf+=sizeof(ATTR)
    read_attribute(tra->stationNum);
    read_attribute(tra->trainID);
    read_attribute(tra->offset);
    read_attribute(tra->seatNum);
    read_attribute(tra->startTime);
    read_attribute(tra->saleDate);
    read_attribute(tra->releaseState);
    read_attribute(tra->type);
    read_attribute(tra->ticket_head);
    read_attribute(tra->ticket_end);
    tra->stations = new char* [tra->stationNum];
    for (int i = 0; i < tra->stationNum; i++) {
        tra->stations[i] = new char[l_han(STATIONS_LEN)];
        memcpy((void*)tra->stations[i],buf,l_han(STATIONS_LEN));buf+=l_han(STATIONS_LEN);
    }
    tra->prices = new int[tra->stationNum];
    tra->travelTimes = new int[tra->stationNum];
    tra->stopoverTimes = new int[tra->stationNum];
    memcpy((void*)tra->prices,buf,sizeof(int)*tra->stationNum);buf+=sizeof(int)*tra->stationNum;
    memcpy((void*)tra->travelTimes,buf,sizeof(int)*tra->stationNum);buf+=sizeof(int)*tra->stationNum;
    memcpy((void*)tra->stopoverTimes,buf,sizeof(int)*tra->stationNum);buf+=sizeof(int)*tra->stationNum;
    int day=calcdays(tra->saleDate/10000, tra->saleDate%10000);
    tra->stationTicketRemains = new int* [day+1];
    for (int i = 0; i <= day; i++) {
        tra->stationTicketRemains[i] = new int[tra->stationNum];
        memcpy((void*)tra->stationTicketRemains[i],buf,sizeof(int)*tra->stationNum);buf+=sizeof(int)*tra->stationNum;
    }

#undef read_attribute
}
void TrainManager::saveTrain(std::fstream& ofs, DiskLoc_T offset, train* tra) {
    ofs.seekp(offset);
    char buffer[trainOffset[offset]];
    char* buf = buffer;
# define write_attribute(ATTR) do{memcpy(buf,(void*)&ATTR,sizeof(ATTR));buf+=sizeof(ATTR);}while(0)
    write_attribute(tra->stationNum);
    write_attribute(tra->trainID);
    write_attribute(tra->offset);
    write_attribute(tra->seatNum);
    write_attribute(tra->startTime);
    write_attribute(tra->saleDate);
    write_attribute(tra->releaseState);
    write_attribute(tra->type);
    write_attribute(tra->ticket_head);
    write_attribute(tra->ticket_end);
    for (int i = 0; i < tra->stationNum; i++) {
        memcpy(buf,(void*)tra->stations[i],l_han(STATIONS_LEN));buf+=l_han(STATIONS_LEN);
    }
    memcpy(buf,(void*)tra->prices,sizeof(int)*tra->stationNum);buf+=sizeof(int)*tra->stationNum;
    memcpy(buf,(void*)tra->travelTimes,sizeof(int)*tra->stationNum);buf+=sizeof(int)*tra->stationNum;
    memcpy(buf,(void*)tra->stopoverTimes,sizeof(int)*tra->stationNum);buf+=sizeof(int)*tra->stationNum;
    for (int i = 0,ed=calcdays(tra->saleDate/10000, tra->saleDate%10000);i<=ed; i++) {
        memcpy(buf,(void*)tra->stationTicketRemains[i],sizeof(int)*tra->stationNum);buf+=sizeof(int)*tra->stationNum;
    }
#undef write_attribute
    ofs.write(buffer, sizeof(buffer));
    tra->~train();
}
int TrainManager::getsize(train* t) {
    int siz = 0;
    siz += sizeof(t->stationNum);
    siz += sizeof(t->trainID);
    siz += sizeof(t->offset);
    siz += sizeof(t->seatNum);
    siz += sizeof(t->startTime);
    siz += sizeof(t->saleDate);
    siz += sizeof(t->releaseState);
    siz += sizeof(t->type);
    siz += sizeof(char)*t->stationNum*l_han(STATIONS_LEN);
    siz += sizeof(int)*t->stationNum;
    siz += sizeof(int)*t->stationNum;
    siz += sizeof(int)*t->stationNum;
    siz += sizeof(t->ticket_end)+sizeof(t->ticket_head);
    siz += sizeof(int)*t->stationNum*(calcdays(t->saleDate/10000, t->saleDate%10000)+1);
    return siz;
}
DiskLoc_T TrainManager::increaseFile(train* tra) {
    DiskLoc_T rt = train_file_size;
    tra->offset = rt;
    trainFile.seekp(train_file_size);
    int siz = getsize(tra);

    char buffer[siz];
    char* buf = buffer;
# define write_attribute(ATTR) do{memcpy(buf,(void*)&ATTR,sizeof(ATTR));buf+=sizeof(ATTR);}while(0)
    write_attribute(tra->stationNum);
    write_attribute(tra->trainID);
    write_attribute(tra->offset);
    write_attribute(tra->seatNum);
    write_attribute(tra->startTime);
    write_attribute(tra->saleDate);
    write_attribute(tra->releaseState);
    write_attribute(tra->type);
    write_attribute(tra->ticket_head);
    write_attribute(tra->ticket_end);
    for (int i = 0; i < tra->stationNum; i++)
        for (int j = 0; j < l_han(STATIONS_LEN); j++) { write_attribute(tra->stations[i][j]); }
    for (int i = 0; i < tra->stationNum; i++) { write_attribute(tra->prices[i]); }
    for (int i = 0; i < tra->stationNum; i++) { write_attribute(tra->travelTimes[i]); }
    for (int i = 0; i < tra->stationNum; i++) { write_attribute(tra->stopoverTimes[i]); }
    for (int i = 0; i <= calcdays(tra->saleDate/10000, tra->saleDate%10000); i++)
        for (int j = 0; j < tra->stationNum; j++) { write_attribute(tra->stationTicketRemains[i][j]); }
#undef write_attribute
    trainFile.write(buffer, sizeof(buffer));

    train_file_size += siz;
    trainOffset[rt] = siz;
    // defaultOut<<rt<<' '<<trainOffset[rt]<<endl;
    return rt;
}

void addtime(int& date, int& tim, int t) {
    int d = t/(24*60);
    t %= (24*60);
    date += d;
    int h = t/60;
    t %= 60;
    int min = t+tim%100;
    tim = (int) (tim/100+min/60+h)*100+min%60;
    if (tim > 2359)tim -= 2400, date++;
    if (date/100 == 6 && date%100 > 30)date = 700+date%100-30;
    if (date/100 == 7 && date%100 > 31)date = 800+date%100-31;
    if (date/100 == 8 && date%100 > 31)date = 900+date%100-31;
}
bool TrainManager::Add_train(const trainID_t& t, int stationNUM, int seatNUM, char** stations,
                             const int* prices, int startTime, const int* travelTimes, const int* stopoverTimes,
                             int saleDate, char type) {//saleDate = mddmdd
    // construct
    if (findtrainID(t)) {
        defaultOut << "-1" << endl;
        return false;
    }
    train tra{};
    strcpy(tra.trainID.ID, t.ID);
    tra.stationNum = stationNUM;
    tra.seatNum = seatNUM;
    tra.startTime = startTime;
    tra.saleDate = saleDate;
    tra.type = type;
    tra.releaseState = false;
    tra.ticket_head = tra.ticket_end = -1;
    tra.stations = new char* [stationNUM];
    for (int i = 0; i < stationNUM; i++) {
        tra.stations[i] = new char[l_han(STATIONS_LEN)];
        strcpy(tra.stations[i], stations[i]);
    }
    tra.prices = new int[stationNUM];
    tra.prices[0] = 0;
    for (int i = 1; i < stationNUM; i++)tra.prices[i] = prices[i-1];
    tra.travelTimes = new int[stationNUM];
    for (int i = 0; i < stationNUM-1; i++)tra.travelTimes[i] = travelTimes[i];
    tra.stopoverTimes = new int[stationNUM];
    for (int i = 1; i < stationNUM-1; i++)tra.stopoverTimes[i] = stopoverTimes[i-1];
    int ___ = 0;
    tra.stationTicketRemains = new int* [calcdays(saleDate/10000, saleDate%10000)+1];
    for (int day = saleDate/10000; day <= saleDate%10000; addtime(day, ___, 24*60)) {
        tra.stationTicketRemains[calcdays(saleDate/10000, day)] = new int[stationNUM];
        for (int i = 0; i < stationNUM-1; i++)tra.stationTicketRemains[calcdays(saleDate/10000, day)][i] = seatNUM;
    }
    // preprocessing
    int date = 0, tim = tra.startTime;
    for (int i = 0; i < tra.stationNum; i++) {
        int tmp = tra.travelTimes[i];
        tra.travelTimes[i] = (i == 0) ? 0 : date*10000+tim;
        if (i != 0 && i != (tra.stationNum)-1)
            addtime(date, tim, tra.stopoverTimes[i]);
        tra.stopoverTimes[i] = (i == (tra.stationNum)-1) ? 0 : date*10000+tim;
        addtime(date, tim, tmp);
        if (i > 0)
            tra.prices[i] += tra.prices[i-1];
    }
    DiskLoc_T off = increaseFile(&tra);
    trainidToOffset.insert(t, off);
    defaultOut << "0" << endl;
    return true;
}
bool TrainManager::Delete_train(const trainID_t& t) {
    if (!findtrainID(t)) {
        defaultOut << "-1" << endl;
        return false;
    }
    DiskLoc_T loc = trainidToOffset.search(t).first;
    auto* ptr = train_cache.get(loc);
    if (ptr->releaseState) {
        defaultOut << "-1" << endl;
        return false;
    }
    //delete
    train_cache.remove(loc);
    trainidToOffset.remove(t);
    defaultOut << "0" << endl;
    return true;
}
bool TrainManager::Release_train(const trainID_t& t) {
    if (!findtrainID(t)) {
        defaultOut << "-1" << endl;
        return false;
    }
    DiskLoc_T loc = trainidToOffset.search(t).first;
    auto* train_ptr = train_cache.get(loc);
    if (train_ptr->releaseState) {
        defaultOut << "-1" << endl;
        return false;
    }
    train_ptr->releaseState = true;
    trainlist.push_back(train_ptr->trainID);
    for (int i = 0; i < train_ptr->stationNum; i++) {
        auto the_station = station_t(train_ptr->stations[i]);
        if (stationlist.find(the_station) == stationlist.end()) {
            assert(train_ptr->stations[i]);
            stationlist[the_station] = ++station_num;
        }
        assert(stationlist[the_station]);
        stationTotrain.insert(stationlist[the_station]*10000+train_num, i);
    }
    train_num++;
    train_cache.set_dirty_bit(loc);
    defaultOut << "0" << endl;
    return true;
}

bool TrainManager::Query_train(const trainID_t& t, int date) {//date = mmdd
    if (!findtrainID(t)) {
        defaultOut << "-1" << endl;
        return false;
    }
    DiskLoc_T loc = trainidToOffset.search(t).first;
    auto* train_ptr = train_cache.get(loc);
    int start = (train_ptr->saleDate)/10000;
    int end = (train_ptr->saleDate)%10000;
    if (date < start || date > end) {
        defaultOut << "-1" << endl;
        return false;
    }
    // print
    defaultOut << (train_ptr->trainID.ID) << ' ' << (train_ptr->type) << endl;
    int ___ = 0;
    int kday = calcdays(start, date);
    for (int i = 0; i < train_ptr->stationNum; i++) {
        defaultOut << (train_ptr->stations[i]) << ' ';
        if (i == 0)defaultOut << "xx-xx xx:xx";
        else printdate(date) << ' ', printtime(getTime(train_ptr->travelTimes[i]));
        defaultOut << " -> ";
        if (i != 0 && i != (train_ptr->stationNum)-1)
            addtime(date, ___, 24*60*(getDate(train_ptr->stopoverTimes[i])-getDate(train_ptr->travelTimes[i])));
        if (i == (train_ptr->stationNum)-1)defaultOut << "xx-xx xx:xx";
        else printdate(date) << ' ', printtime(getTime(train_ptr->stopoverTimes[i]));
        if (i != train_ptr->stationNum-1)
            addtime(date, ___, 24*60*(getDate(train_ptr->travelTimes[i+1])-getDate(train_ptr->stopoverTimes[i])));
        if (i == 0)
            defaultOut << " 0";
        else
            defaultOut << ' ' << (train_ptr->prices[i]);
        if (i == (train_ptr->stationNum)-1)
            defaultOut << " x" << endl;
        else
            defaultOut << ' ' << (train_ptr->stationTicketRemains[kday][i]) << endl;
    }
    return true;
}

void TrainManager::print_ticket(const pair<int, pair<int, int>>& A, int date) {
    trainID_t tra = trainlist[A.first];
    DiskLoc_T loc = trainidToOffset.search(tra).first;
    auto* train_ptr = train_cache.get(loc);
    int s = A.second.first/100, t = A.second.first%100;
    int days = getDate(train_ptr->stopoverTimes[s]);
    int startday = calcstartday(date, days);
    int start = (train_ptr->saleDate)/10000;
    int end = (train_ptr->saleDate)%10000;

    defaultOut << (train_ptr->trainID.ID) << ' ' << train_ptr->stations[s] << ' ';
    printdate(date) << ' ';
    printtime(getTime(train_ptr->stopoverTimes[s])) << " -> " << train_ptr->stations[t] << ' ';
    int tdate = date, tmp = 0;
    addtime(tdate, tmp, 24*60*(int) (getDate(train_ptr->travelTimes[t])-getDate(train_ptr->stopoverTimes[s])));
    printdate(tdate) << ' ';
    printtime(getTime(train_ptr->travelTimes[t])) << ' ' << (train_ptr->prices[t]-train_ptr->prices[s]) << ' ';
    int seat = train_ptr->seatNum;
    for (int j = s; j < t; j++)seat = std::min(seat, train_ptr->stationTicketRemains[calcdays(start, startday)][j]);
    defaultOut << seat << endl;
}

bool TrainManager::Query_ticket(const char* Sstation, const char* Tstation, int date, int order) {
    if (stationlist.find(station_t(Sstation)) == stationlist.end() ||
        stationlist.find(station_t(Tstation)) == stationlist.end()) {
        defaultOut << "0" << endl;
        return false;
    }
    assert(stationlist[station_t(Sstation)]);
    assert(stationlist[station_t(Tstation)]);
    ds::vector<pair<long long, int>> S = stationTotrain.range(stationlist[station_t(Sstation)]*10000LL,
                                                              stationlist[station_t(Sstation)]*10000LL+9999);
    ds::vector<pair<long long, int>> T = stationTotrain.range(stationlist[station_t(Tstation)]*10000LL,
                                                              stationlist[station_t(Tstation)]*10000LL+9999);
    ds::vector<pair<int, pair<int, int>>> Ans;  // {train_id,{k1th_station*100+k2th_station,key}}

    int ansnum = 0;
    for (int i = 0, j = 0; i < S.size() && j < T.size();) {
        if (train_id_f(S[i].first) < train_id_f(T[j].first))i++;
        else if (train_id_f(S[i].first) > train_id_f(T[j].first))j++;
        else {
            if (S[i].second < T[j].second) {
                trainID_t t = trainlist[train_id_f(S[i].first)];
                DiskLoc_T loc = trainidToOffset.search(t).first;
                auto* train_ptr = train_cache.get(loc);
                int days = getDate(train_ptr->stopoverTimes[S[i].second]);
                int startday = calcstartday(date, days);
                int start = (train_ptr->saleDate)/10000;
                int end = (train_ptr->saleDate)%10000;
                if (startday >= start && startday <= end) {
                    // on sale
                    int key = (order == TIME) ?
                              calctime(train_ptr->stopoverTimes[S[i].second],
                                       train_ptr->travelTimes[T[j].second]) //time = dddhhmm
                                              : train_ptr->prices[T[j].second]-train_ptr->prices[S[i].second];
                    pair<int, pair<int, int>> Element;
                    Element.first = (int) train_id_f(S[i].first);
                    Element.second.first = S[i].second*100+T[j].second;
                    Element.second.second = key;
                    ansnum++;
                    Ans.push_back(Element);
                }
            }
            i++, j++;
        }
    }
    defaultOut << ansnum << endl;
    if (ansnum == 0)return false;
    static const int maxN = 10000;
    static int remap[maxN];
    for (int i = 0; i < ansnum; i++)remap[i] = i;
    for (auto e:Ans) {
        assert(e.first < trainlist.size());
    }
    auto cmp = [&Ans, this](int x, int y) -> bool {
        return (Ans[x].second.second == Ans[y].second.second) ?
               strcmp(trainlist[Ans[x].first].ID, trainlist[Ans[y].first].ID) < 0
                                                              : Ans[x].second.second < Ans[y].second.second;
    };
    std::sort(remap, remap+ansnum, cmp);
    // print tickets
    for (int i = 0; i < ansnum; ++i) {
        print_ticket(Ans[remap[i]], date);
    }
    return true;
}
bool TrainManager::Query_transfer(const char* Sstation, const char* Tstation, int date, int order) {
    if (stationlist.find(station_t(Sstation)) == stationlist.end() ||
        stationlist.find(station_t(Tstation)) == stationlist.end()) {
        defaultOut << "0" << endl;
        return false;
    }
    ds::vector<pair<long long, int>> S = stationTotrain.range(stationlist[station_t(Sstation)]*10000LL,
                                                              stationlist[station_t(Sstation)]*10000LL+9999);
    ds::vector<pair<long long, int>> T = stationTotrain.range(stationlist[station_t(Tstation)]*10000LL,
                                                              stationlist[station_t(Tstation)]*10000LL+9999);
    int minkey = INT32_MAX,min_run_time=INT32_MAX;
    pair<int, pair<int, int>> A, B; // {train,{stat_s*100+stat_t,date}}
    for (int i = 0; i < S.size(); i++)
        for (int j = 0; j < T.size(); j++)
            if (train_id_f(S[i].first) != train_id_f(T[j].first)) {
                trainID_t t1 = trainlist[train_id_f(S[i].first)], t2 = trainlist[train_id_f(T[j].first)];
                DiskLoc_T loc = trainidToOffset.search(t1).first, loc2 = trainidToOffset.search(t2).first;
                auto* tra_1_ptr = train_cache.get(loc);
                auto* tra_2_ptr = train_cache.get(loc2);
                int days = getDate(tra_1_ptr->stopoverTimes[S[i].second]);
                int startday = calcstartday(date, days);
                int first_train_start_date = (tra_1_ptr->saleDate)/10000;
                int first_train_end_date = (tra_1_ptr->saleDate)%10000;
                int second_train_start_date = (tra_2_ptr->saleDate)/10000;
                int second_train_end_date = (tra_2_ptr->saleDate)%10000;
                // check s_train date
                if (startday < first_train_start_date || startday > first_train_end_date)continue;
                //check_transfer
                int first_kth_station = S[i].second, second_kth_station = T[j].second;
                for (int mid_stat_tra_1 = first_kth_station+1;
                     mid_stat_tra_1 < tra_1_ptr->stationNum; mid_stat_tra_1++) {
                    for (int mid_stat_tra_2 = 0;
                         mid_stat_tra_2 < second_kth_station; mid_stat_tra_2++) {
                        // for every station in train_1st, check if it exists in train_2nd
                        if (strcmp(tra_1_ptr->stations[mid_stat_tra_1],
                                   tra_2_ptr->stations[mid_stat_tra_2]) == 0) {
                            // T_arrive <= T_leave
                            int stopoverday = 0;
                            if (getTime(tra_1_ptr->travelTimes[mid_stat_tra_1]) >
                                getTime(tra_2_ptr->stopoverTimes[mid_stat_tra_2]))
                                stopoverday = 1;
                            // check t_train date
                            int second_leave_day = startday, ___ = 0;
                            addtime(second_leave_day, ___,
                                    24*60*(getDate(tra_1_ptr->travelTimes[mid_stat_tra_1])+stopoverday));
                            int days2 = getDate(tra_2_ptr->stopoverTimes[mid_stat_tra_2]);
                            int beststartday2 = calcstartday(second_leave_day, days2); // 2th train earliest leave day
                            if (beststartday2 > second_train_end_date)break;
                            int interval_day = stopoverday;
                            if (beststartday2 < second_train_start_date) {
                                // wait until second train can leave
                                interval_day = second_train_start_date-beststartday2+stopoverday;
                                second_leave_day = second_train_start_date;
                                addtime(second_leave_day, ___,
                                        24*60*getDate(tra_2_ptr->stopoverTimes[mid_stat_tra_2]));
                            }
                            int key;
                            int first_run_time=(min_fmt(tra_1_ptr->travelTimes[mid_stat_tra_1])
                                                -min_fmt(tra_1_ptr->stopoverTimes[first_kth_station]));// first train time
                            if (order == TIME) {
                                int wait_min=min_fmt(getTime(tra_2_ptr->stopoverTimes[mid_stat_tra_2]))
                                        -min_fmt(getTime(tra_1_ptr->travelTimes[mid_stat_tra_1]))
                                        +24*60*interval_day;
                                key = first_run_time+wait_min
                                      +(min_fmt(tra_2_ptr->travelTimes[second_kth_station])
                                      -min_fmt(tra_2_ptr->stopoverTimes[mid_stat_tra_2])); // second train time
                            } else {
                                key = tra_1_ptr->prices[mid_stat_tra_1]-
                                      tra_1_ptr->prices[first_kth_station]+
                                      tra_2_ptr->prices[second_kth_station]-
                                      tra_2_ptr->prices[mid_stat_tra_2];
                            }
                            auto cmp_id_then_run_time=[tra_1_ptr, this,first_run_time,min_run_time,&S,i]()->bool {
                                int rt=strcmp(tra_1_ptr->trainID.ID,trainlist[train_id_f(S[i].first)].ID);
                                return rt!=0?rt<0:first_run_time<min_run_time;
                            };
                            if (key < minkey||(key==minkey&&cmp_id_then_run_time())) {
                                minkey = key;
                                min_run_time=first_run_time;
                                assert(strcmp(tra_1_ptr->stations[mid_stat_tra_1],
                                              tra_2_ptr->stations[mid_stat_tra_2]) == 0);
                                A = std::make_pair((int) train_id_f(S[i].first),
                                                   std::make_pair(first_kth_station*100+mid_stat_tra_1, date));
                                B = std::make_pair((int) train_id_f(T[j].first),
                                                   std::make_pair(mid_stat_tra_2*100+second_kth_station,
                                                                  second_leave_day));
                            }
                            break;
                        }
                    }
                }
            }
    // print
    if (minkey == INT32_MAX) {
        defaultOut << 0 << endl;
        return false;
    } else {
        print_ticket(A, A.second.second);
        print_ticket(B, B.second.second);
        return true;
    }
}

bool
TrainManager::Buy_ticket(UserManager* usr_manager, UserOrderManager* ord_manager, PendingTicketManager* pend_manager,
                         username_t usr, trainID_t tra,
                         int date, int num, char* Sstation, char* Tstation, bool wait) {
    //check
    if (!usr_manager->isOnline(usr) || !findtrainID(tra) || strcmp(Sstation, Tstation) == 0) {
        defaultOut << "-1" << endl;
        return false;
    }
    DiskLoc_T loc = trainidToOffset.search(tra).first;
    auto* train_ptr = train_cache.get(loc);
    if (!train_ptr->releaseState || num > train_ptr->seatNum) {
        defaultOut << "-1" << endl;
        return false;
    }
    int s = -1, t = -1;
    for (int i = 0; i < train_ptr->stationNum; i++) {
        if (strcmp(train_ptr->stations[i], Sstation) == 0) {
            s = i;
        }
        if (strcmp(train_ptr->stations[i], Tstation) == 0) {
            t = i;
            break;
        }
    }
    if (s == -1 || t == -1 || s >= t) {
        defaultOut << "-1" << endl;
        return false;
    }
    int days = getDate(train_ptr->stopoverTimes[s]),
            startday = calcstartday(date, days),
            start = (train_ptr->saleDate)/10000,
            end = (train_ptr->saleDate)%10000;
    if (startday < start || startday > end) {
        defaultOut << "-1" << endl;
        return false;
    }
    int day = calcdays(start, startday);
    //success?
    int seat = train_ptr->seatNum;
    for (int j = s; j < t; j++)seat = std::min(seat, train_ptr->stationTicketRemains[day][j]);
    if (seat >= num) {
        for (int j = s; j < t; j++)train_ptr->stationTicketRemains[day][j] -= num;
        //defaultOut << s<<' '<<t << endl;
        defaultOut << 1ll*num*(train_ptr->prices[t]-train_ptr->prices[s]) << endl;
        order Order;
        Order.stat = order::SUCCESS;
        Order.leaveTime = date*10000+getTime(train_ptr->stopoverTimes[s]);
        int arrdate = date, tmp = 0;
        addtime(arrdate, tmp, 24*60*(getDate(train_ptr->travelTimes[t])-getDate(train_ptr->stopoverTimes[s])));
        Order.arriveTime = arrdate*10000+getTime(train_ptr->travelTimes[t]);
        Order.price = train_ptr->prices[t]-train_ptr->prices[s];
        Order.num = num;
        Order.day = day;
        Order.key = ++ticket_num;
        strcpy(Order.trainID, tra.ID);
        strcpy(Order.from, Sstation);
        strcpy(Order.to, Tstation);
        usr_manager->addorder(ord_manager, usr, &Order);
        train_cache.set_dirty_bit(loc);
    } else {
        if (wait == 0) {
            defaultOut << "-1" << endl;
            return false;
        } else {
            defaultOut << "queue" << endl;
            order Order;
            Order.stat = order::PENDING;
            Order.leaveTime = date*10000+getTime(train_ptr->stopoverTimes[s]);
            int arrdate = date, tmp = 0;
            addtime(arrdate, tmp, 24*60*(getDate(train_ptr->travelTimes[t])-getDate(train_ptr->stopoverTimes[s])));
            Order.arriveTime = arrdate*10000+getTime(train_ptr->travelTimes[t]);
            Order.price = train_ptr->prices[t]-train_ptr->prices[s];
            Order.num = num;
            Order.day = day;
            Order.key = ++ticket_num;
            strcpy(Order.trainID, tra.ID);
            strcpy(Order.from, Sstation);
            strcpy(Order.to, Tstation);
            auto where = usr_manager->addorder(ord_manager, usr, &Order);
            pending_order porder;
            porder.day = day;
            porder.key = Order.key;
            porder.block = where.first;
            porder.nxt = -1;
            porder.num = Order.num;
            porder.s = s;
            porder.t = t;
            porder.offset_in_block = where.second;
            pend_manager->add_pendingorder(&porder, train_ptr);
            train_cache.set_dirty_bit(loc);
        }
    };
    return true;
}
bool
TrainManager::Refund_ticket(UserManager* usr_manager, UserOrderManager* ord_manager, PendingTicketManager* pend_manager,
                            username_t usr, int x) {
    //check
    if (!usr_manager->isOnline(usr)) {
        defaultOut << "-1" << endl;
        return false;
    }
    auto getorder_result = usr_manager->refund_and_return_order(ord_manager, usr, x);
    if (!getorder_result.first) {
        defaultOut << "-1" << endl;
        return false;
    }
    defaultOut << "0" << endl;
    order& Order = (getorder_result.second);
    DiskLoc_T train_offset = trainidToOffset.search(trainID_t(Order.trainID)).first;
    auto* train_ptr = train_cache.get(train_offset);
    int s = -1, t = -1;
    for (int i = 0; i < train_ptr->stationNum; i++) {
        if (strcmp(train_ptr->stations[i], Order.from) == 0) {
            s = i;
        }
        if (strcmp(train_ptr->stations[i], Order.to) == 0) {
            t = i;
            break;
        }
    }
    if (Order.stat == order::SUCCESS) {
        for (int j = s; j < t; j++)train_ptr->stationTicketRemains[Order.day][j] += Order.num;
        pend_manager->allocate_tickets(ord_manager, train_ptr, &Order);
    } else {
        pend_manager->cancel_pending(Order.key, train_ptr);
    }
    train_cache.set_dirty_bit(train_offset);
    return true;
}
TrainManager::TrainManager(const std::string& file_path, const std::string& trainid_index_path,
                           const std::string& station_index_path, const std::string& train_info_path,
                           const std::string& station_info_path, const std::string& offset_info_path)
        : train_cache(353, [this](DiskLoc_T off, train* tra) {
                          assert(trainFile.good());
                          loadTrain(trainFile, off, tra);
                      },
                      [this](DiskLoc_T off, train* tra) {
                          assert(trainFile.good());
                          saveTrain(trainFile, off, tra);
                      },0.2),
          trainidToOffset(trainid_index_path, 87),
          stationTotrain(station_index_path, 127),
          defaultOut(std::cout), train_info_path(train_info_path), station_info_path(station_info_path),
          offset_info_path(offset_info_path) {
    trainFile.open(file_path);
    //metadata
    char buf[sizeof(train_file_size)+sizeof(int)*3];
    char* ptr = buf;
    trainFile.seekg(0);
    trainFile.read(buf, sizeof(buf));
#define read_attribute(ATTR) memcpy((void*)&ATTR,ptr,sizeof(ATTR));ptr+=sizeof(ATTR)
    read_attribute(train_file_size);
    read_attribute(train_num);
    read_attribute(station_num);
    read_attribute(ticket_num);
#undef read_attribute
    ds::SerialVector<trainID_t>::deserialize(trainlist, train_info_path);
    ds::SerialMap<station_t, int>::deserialize(stationlist, station_info_path);
    ds::SerialMap<DiskLoc_T, int>::deserialize(trainOffset, offset_info_path);
}

TrainManager::~TrainManager() {
    // write metadata
    char buf[sizeof(train_file_size)+sizeof(int)*3];
    char* ptr = buf;
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
    write_attribute(train_file_size);
    write_attribute(train_num);
    write_attribute(station_num);
    write_attribute(ticket_num);
#undef write_attribute
    assert(trainFile.good());
    trainFile.seekp(0);
    trainFile.write(buf, sizeof(buf));
    train_cache.destruct();
    trainFile.close();
    ds::SerialVector<trainID_t>::serialize(trainlist, train_info_path);
    ds::SerialMap<station_t, int>::serialize(stationlist, station_info_path);
    ds::SerialMap<DiskLoc_T, int>::serialize(trainOffset, offset_info_path);
}


void TrainManager::Init(const std::string& file_path) {
    std::fstream f(file_path, ios::out | ios::binary);
    char buf[sizeof(DiskLoc_T)+sizeof(int)*3];
    char* ptr = buf;
    DiskLoc_T sz = sizeof(buf);
    int train_num = 0, station_num = 0, ticket_num = 0;
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
    write_attribute(sz);
    write_attribute(train_num);
    write_attribute(station_num);
    write_attribute(ticket_num);
#undef write_attribute
    f.write(buf, sizeof(buf));
    f.close();

}
