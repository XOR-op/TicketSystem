#include "TrainManager.h"

using namespace t_sys;
int getDate(const int x){
    return x/10000;
}

int getTime(const int x){
    return x%10000;
}
static void loadTrain(std::fstream& ifs,DiskLoc_T offset,train* tra){
    ifs.seekg(offset);
    ifs.read((char*)tra, sizeof(train));
}
static void saveTrain(std::fstream& ofs,DiskLoc_T offset,const train* tra){
    ofs.seekp(offset);
    ofs.write((char*)tra, sizeof(train));
}

DiskLoc_T TrainManager::increaseFile(train* tra) {
    if (head == NULL) {
        DiskLoc_T rt = train_file_size;
        tra->offset = rt;
        trainFile.seekp(train_file_size);
        trainFile.write((char*) tra, sizeof(train));
        train_file_size += sizeof(train);
        return rt;
    } else {
        DiskLoc_T rt = head->pos;
        tra->offset = rt;
        trainFile.seekp(head->pos);
        trainFile.write((char*) tra, sizeof(train));
        head = head->nxt;
        return rt;
    }
}

void TrainManager::addtime(int& date, int& tim, int t) {
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
    for (int i = 0; i < stationNUM; i++)strcpy(tra.stations[i], stations[i]);
    for (int i = 1; i < stationNUM; i++)tra.prices[i] = prices[i-1];
    for (int i = 0; i < stationNUM-1; i++)tra.travelTimes[i] = travelTimes[i];
    for (int i = 1; i < stationNUM-1; i++)tra.stopoverTimes[i] = stopoverTimes[i-1];
    int tmp = 0;
    for (int day = saleDate/10000; day <= saleDate%10000; addtime(day, tmp, 24*60))
        for (int i = 0; i < stationNUM-1; i++)tra.stationTicketRemains[calcdays(saleDate/10000, day)][i] = seatNUM;
    // write back
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
    //add to freenode
    auto* new_node = new freenode;
    new_node->nxt = head;
    new_node->pos = loc;
    head = new_node;
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
        auto the_station=station_t(train_ptr->stations[i]);
        if (stationlist.find(the_station) == stationlist.end()) {
            assert(train_ptr->stations[i]);
            stationlist[the_station] = ++station_num;
        }
        //std::cout<<i<<' '<<stationlist[station_t(train_ptr->stations[1])]<<endl;
        stationTotrain.insert(stationlist[the_station]*10000+train_num, i);
    }
    train_num++;
    int date = 0, tim = train_ptr->startTime;
    for (int i = 0; i < train_ptr->stationNum; i++) {
        int tmp = train_ptr->travelTimes[i];
        train_ptr->travelTimes[i] =(i==0)?0: date*10000+tim;
        if (i != 0 && i != (train_ptr->stationNum)-1)
            addtime(date, tim, train_ptr->stopoverTimes[i]);
        train_ptr->stopoverTimes[i] =(i == (train_ptr->stationNum)-1)?0: date*10000+tim;
        addtime(date, tim, tmp);
        if (i > 0)
            train_ptr->prices[i] += train_ptr->prices[i-1];
    }
    //maybe more things need to do
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
    defaultOut << (train_ptr->trainID.ID) << ' ' << (train_ptr->type) << endl;
    int tmp = 0;
    int kday = calcdays(start, date);
    for (int i = 0; i < train_ptr->stationNum; i++) {
        defaultOut << (train_ptr->stations[i]) << ' ';
        if (i == 0)defaultOut << "xx-xx xx:xx";
        else printdate(date), defaultOut << ' ', printtime(getTime(train_ptr->travelTimes[i]));
        defaultOut << " -> ";
        if (i != 0 && i != (train_ptr->stationNum)-1)
            addtime(date, tmp, 24*60*(getDate(train_ptr->stopoverTimes[i])-getDate(train_ptr->travelTimes[i])));
        if (i == (train_ptr->stationNum)-1)defaultOut << "xx-xx xx:xx";
        else printdate(date), defaultOut << ' ', printtime(getTime(train_ptr->stopoverTimes[i]));
        addtime(date, tmp, 24*60*(getDate(train_ptr->travelTimes[i+1])-getDate(train_ptr->stopoverTimes[i])));
        if (i == 0)defaultOut << " 0";
        else defaultOut << ' ' << (train_ptr->prices[i]);
        if (i == (train_ptr->stationNum)-1)defaultOut << " x" << endl;
        else defaultOut << ' ' << (train_ptr->stationTicketRemains[kday][i]) << endl;
    }
    return true;
}
bool TrainManager::Query_ticket(const char* Sstation, const char* Tstation, int date, int order) {
    pse_std::vector<std::pair<long long, int>> S = stationTotrain.range(stationlist[station_t(Sstation)]*10000LL,
                                                                        stationlist[station_t(Sstation)]*10000LL+9999);
    pse_std::vector<std::pair<long long, int>> T = stationTotrain.range(stationlist[station_t(Tstation)]*10000LL,
                                                                        stationlist[station_t(Tstation)]*10000LL+9999);
    pse_std::vector<std::pair<int, std::pair<int, int>>> Ans;
    int ansnum = 0;
    for (int i = 0, j = 0; i < S.size() && j < T.size();) {
        if (S[i].first%10000 < T[j].first%10000)i++;
        else if (S[i].first%10000 > T[j].first%10000)j++;
        else {
            if (S[i].second < T[j].second) {
                trainID_t t = trainlist[S[i].first%10000];
                DiskLoc_T loc = trainidToOffset.search(t).first;
                auto* train_ptr = train_cache.get(loc);
                int days = getDate(train_ptr->stopoverTimes[S[i].second]);
                int startday = calcstartday(date, days);
                int start = (train_ptr->saleDate)/10000;
                int end = (train_ptr->saleDate)%10000;
                if (startday >= start && startday <= end) {
                    int key;
                    if (order == 0) {
                        key = calctime(train_ptr->stopoverTimes[S[i].second], train_ptr->travelTimes[T[j].second]);//time = dddhhmm
                    } else {
                        key = train_ptr->prices[T[j].second]-train_ptr->prices[S[i].second];
                    }
                    std::pair<int, std::pair<int, int>> Element;
                    Element.first = (int) (S[i].first%10000);
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
    static int s[maxN];
    for (int i = 0; i < ansnum; i++)s[i] = i;
    auto cmp = [&Ans](int x, int y) -> bool { return Ans[x].second.second < Ans[y].second.second; };
    std::sort(s, s+ansnum, cmp);
    //defaultOut
    for (int i = 0; i < ansnum; i++) {
        int p = s[i];
        trainID_t tra = trainlist[Ans[p].first];
        DiskLoc_T loc = trainidToOffset.search(tra).first;
        auto* train_ptr = train_cache.get(loc);
        int s = Ans[p].second.first/100, t = Ans[p].second.first%100;
        int days = getDate(train_ptr->stopoverTimes[s]);
        int startday = calcstartday(date, days);
        int start = (train_ptr->saleDate)/10000;
        int end = (train_ptr->saleDate)%10000;

        defaultOut << (train_ptr->trainID.ID) << ' ' << Sstation << ' ';
        printdate(date);
        defaultOut << ' ';
        printtime(getTime(train_ptr->stopoverTimes[s]));
        defaultOut << " -> " << Tstation << ' ';
        int tdate = date, tmp = 0;
        addtime(tdate, tmp, 24*60*(getDate(train_ptr->travelTimes[t])-getDate(train_ptr->stopoverTimes[s])));
        printdate(tdate);
        defaultOut << ' ';
        printtime(getTime(train_ptr->travelTimes[t]));
        defaultOut << ' ' << (train_ptr->prices[t]-train_ptr->prices[s]) << ' ';
        int seat = train_ptr->seatNum;
        for (int j = s; j < t; j++)seat = std::min(seat, train_ptr->stationTicketRemains[calcdays(start, startday)][j]);
        defaultOut << seat << endl;
    }
    return true;
}
void TrainManager::transfer_sub_print(const std::pair<int, std::pair<int, int>>& A,int date,const char* station){
    trainID_t tra = trainlist[A.first];
    DiskLoc_T loc = trainidToOffset.search(tra).first;
    auto* train_ptr = train_cache.get(loc);
    int s = A.second.first/100, t = A.second.first%100;
    int days = getDate(train_ptr->stopoverTimes[s]);
    int startday = calcstartday(date, days);
    int start = (train_ptr->saleDate)/10000;
    int end = (train_ptr->saleDate)%10000;
    defaultOut << (train_ptr->trainID.ID) << ' ' << station << ' ';
    printdate(date);
    defaultOut << ' ';
    printtime(getTime(train_ptr->stopoverTimes[s]));
    defaultOut << " -> " << train_ptr->stations[t] << ' ';
    int tdate = date, tmp = 0;
    addtime(tdate, tmp, 24*60*(int) (getDate(train_ptr->travelTimes[t])-getDate(train_ptr->stopoverTimes[s])));
    printdate(tdate);
    defaultOut << ' ';
    printtime(getTime(train_ptr->travelTimes[t]));
    defaultOut << ' ' << (train_ptr->prices[t]-train_ptr->prices[s]) << ' ';
    int seat = train_ptr->seatNum;
    for (int j = s; j < t; j++)seat = std::min(seat, train_ptr->stationTicketRemains[calcdays(start, startday)][j]);
    defaultOut << seat << endl;
}
bool TrainManager::Query_transfer(const char* Sstation, const char* Tstation, int date, int order) {
    pse_std::vector<std::pair<long long, int>> S = stationTotrain.range(stationlist[station_t(Sstation)]*10000LL,
                                                                        stationlist[station_t(Sstation)]*10000LL+9999);
    pse_std::vector<std::pair<long long, int>> T = stationTotrain.range(stationlist[station_t(Tstation)]*10000LL,
                                                                        stationlist[station_t(Tstation)]*10000LL+9999);
    int minkey = 2147483647;
    std::pair<int, std::pair<int, int>> A, B;
    for (int i = 0; i < S.size(); i++)
        for (int j = 0; j < T.size(); j++)
            if (S[i].first%10000 != T[j].first%10000) {
                //printf("%d %d %lld %lld\n",i,j,S[i].first,T[j].first);
                trainID_t t1 = trainlist[S[i].first%10000], t2 = trainlist[T[j].first%10000];
                DiskLoc_T loc = trainidToOffset.search(t1).first, loc2 = trainidToOffset.search(t2).first;
                auto* train_ptr = train_cache.get(loc);
                auto* train_ptr2 = train_cache.get(loc2);
                int days = getDate(train_ptr->stopoverTimes[S[i].second]);
                int startday = calcstartday(date, days);
                int start = (train_ptr->saleDate)/10000;
                int end = (train_ptr->saleDate)%10000;
                int start2 = (train_ptr2->saleDate)/10000;
                int end2 = (train_ptr2->saleDate)%10000;
                //s_train date is ok?
                if (startday < start || startday > end)continue;
                //check_transfer
                int station1 = S[i].second, station2 = T[j].second;
                for (int k = station1+1; k < train_ptr->stationNum; k++) {
                    for (int h = 0; h < station2; h++)
                        if (strcmp(train_ptr->stations[k], train_ptr2->stations[h]) == 0) {
                            //printf("%d %d\n",k,h);
                            //T_arrive <= T_leave
                            int stopoverday = 0;
                            if (getTime(train_ptr->travelTimes[k]) > getTime(train_ptr2->stopoverTimes[h]))stopoverday = 1;
                            //t_train date is ok?
                            int hdate = startday, tmp = 0;
                            addtime(hdate, tmp, 24*60*(getDate(train_ptr->travelTimes[k])+stopoverday));
                            int days2 = getDate(train_ptr2->stopoverTimes[h]);
                            int beststartday2 = calcstartday(hdate, days2);
                            if (beststartday2 > end2)break;
                            int key;
                            if (beststartday2 >= start2) {
                                if (order == 0) {
                                    key = calctime(train_ptr->stopoverTimes[station1], train_ptr->travelTimes[k])+
                                          calctime(getTime(train_ptr->travelTimes[k]), 10000*stopoverday+train_ptr2->stopoverTimes[h])+
                                          calctime(train_ptr2->stopoverTimes[h], train_ptr2->travelTimes[station2]);
                                } else {
                                    key = train_ptr->prices[k]-train_ptr->prices[station1]+train_ptr2->prices[station2]-train_ptr2->prices[h];
                                }
                                if (key < minkey) {
                                    minkey = key;
                                    A = std::make_pair((int) (S[i].first%10000), std::make_pair(station1*100+k, date));
                                    B = std::make_pair((int) (T[j].first%10000), std::make_pair(h*100+station2, hdate));
                                }
                            } else {
                                hdate = start2;
                                int tmp = 0;
                                addtime(hdate, tmp, 24*60*getDate(train_ptr2->stopoverTimes[h]));
                                if (order == 0) {
                                    key = calctime(train_ptr->stopoverTimes[station1], train_ptr->travelTimes[k])+
                                          calctime(getTime(train_ptr->travelTimes[k]),
                                                   10000*(start2-beststartday2+stopoverday)+train_ptr2->stopoverTimes[h])+
                                          calctime(train_ptr2->stopoverTimes[h], train_ptr2->travelTimes[station2]);
                                } else {
                                    key = train_ptr->prices[k]-train_ptr->prices[station1]+train_ptr2->prices[station2]-train_ptr2->prices[h];
                                }
                                if (key < minkey) {
                                    minkey = key;
                                    A = std::make_pair((int) (S[i].first%10000), std::make_pair(station1*100+k, date));
                                    B = std::make_pair((int) (T[j].first%10000), std::make_pair(h*100+station2, hdate));
                                }
                            }
                            break;
                        }
                }
            }
    //defaultOut
    if (minkey == 2147483647) {
        defaultOut << 0 << endl;
        return false;
    } else {
        transfer_sub_print(A,date,Sstation);
        transfer_sub_print(B,date,Tstation);
        return true;
    }
}
bool TrainManager::Buy_ticket(UserManager* usr_manager, UserOrderManager* ord_manager,PendingTicketManager* pend_manager, username_t usr, trainID_t tra,
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
            porder.num= Order.num;
            porder.s = s;
            porder.t = t;
            porder.offset_in_block = where.second;
            pend_manager->add_pendingorder(&porder, train_ptr);
            train_cache.set_dirty_bit(loc);
        }
    };
    return true;
}
bool TrainManager::Refund_ticket(UserManager* usr_manager, UserOrderManager* ord_manager,PendingTicketManager* pend_manager, username_t usr, int x) {
    //check
    if (!usr_manager->isOnline(usr)) {
        defaultOut << "-1" << endl;
        return false;
    }
    auto getorder_result = usr_manager->getorder(ord_manager, usr, x);
    if (!getorder_result.first) {
        defaultOut << "-1" << endl;
        return false;
    }
    defaultOut << "0" << endl;
    order& Order = (getorder_result.second);
    DiskLoc_T loc = trainidToOffset.search(trainID_t(Order.trainID)).first;
    auto* ptr = train_cache.get(loc);
    int s = -1, t = -1;
    for (int i = 0; i < ptr->stationNum; i++) {
        if (strcmp(ptr->stations[i], Order.from) == 0) {
            s = i;
        }
        if (strcmp(ptr->stations[i], Order.to) == 0) {
            t = i;
            break;
        }
    }
    if (Order.stat == order::SUCCESS) {
        for (int j = s; j < t; j++)ptr->stationTicketRemains[Order.day][j] += Order.num;
        pend_manager->allocate_tickets(ord_manager, ptr, &Order);
    } else {
        pend_manager->cancel_pending(Order.key,ptr);
    }
    train_cache.set_dirty_bit(loc);
    return true;
}
TrainManager::TrainManager(const std::string& file_path,
                           const std::string& trainid_index_path, const std::string& station_index_path)
        : train_cache(51, [this](DiskLoc_T off, train* tra) {
            assert(trainFile.good());
            loadTrain(trainFile, off, tra); },
                      [this](DiskLoc_T off, const train* tra) {
            assert(trainFile.good());
            saveTrain(trainFile, off, tra); }),
          trainidToOffset(trainid_index_path, 107),
          stationTotrain(station_index_path, 157),
          head(NULL), defaultOut(std::cout), train_num(0), station_num(0), ticket_num(0) {
    trainFile.open(file_path);
    //metadata
    char buf[sizeof(train_file_size)];
    char* ptr = buf;
    trainFile.seekg(0);
    trainFile.read(buf, sizeof(buf));
#define read_attribute(ATTR) memcpy((void*)&ATTR,ptr,sizeof(ATTR));ptr+=sizeof(ATTR)
    read_attribute(train_file_size);
#undef read_attribute
}

TrainManager::~TrainManager() {
    // write metadata
    char buf[sizeof(train_file_size)];
    char* ptr = buf;
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
    write_attribute(train_file_size);
#undef write_attribute
    assert(trainFile.good());
    trainFile.seekp(0);
    trainFile.write(buf, sizeof(buf));
    train_cache.destruct();
    trainFile.close();
}








int TrainManager::calcstartday(int date, int days) {
    if (date%100 > days)return date-days;
    days -= date%100;
    if (date/100 == 9)return calcstartday(831, days);
    if (date/100 == 8)return calcstartday(731, days);
    if (date/100 == 7)return calcstartday(630, days);
    if (date/100 == 6)return calcstartday(531, days);
    return -1;
}
int TrainManager::calctime(int start, int end) {
    int mm = end%100-start%100;
    start /= 100, end /= 100;
    if (mm < 0)mm += 60, end--;
    int hh = end/100-start/100;
    start /= 100, end /= 100;
    if (hh < 0)hh += 24, end--;
    int ddd = end-start;
    return ddd*10000+hh*100+mm;
}
int TrainManager::calcdays(int start, int end) {
    if (start/100 == end/100)return end-start;
    else {
        int ans = 0;
        if (start/100 == 6)ans = 30-start%100;
        else ans = 31-start%100;
        for (int i = start/100+1; i < end/100; i++) {
            if (i == 7 || i == 8 || i == 10)ans += 31;
            else ans += 30;
        }
        return ans+end%100;
    }
}
unsigned int TrainManager::myhash(const void* key, int len, unsigned int seed) {
    const unsigned int m = 0x5bd1e995;
    const int r = 24;
    unsigned int h = seed ^len;
    const unsigned char* data = (const unsigned char*) key;
    while (len >= 4) {
        unsigned int k = *(unsigned int*) data;
        k *= m;
        k ^= k >> r;
        k *= m;
        h *= m;
        h ^= k;
        data += 4;
        len -= 4;
    }
    switch (len) {
        case 3:
            h ^= data[2] << 16;
        case 2:
            h ^= data[1] << 8;
        case 1:
            h ^= data[0];
            h *= m;
    };
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    return h;
}
int TrainManager::findtrainID(const trainID_t& t) {
    auto rt = trainidToOffset.search(t);
    if (rt.second)return 1;
    else return 0;
}
void TrainManager::print(int x) {
    if (x < 10)defaultOut << '0' << x;else defaultOut << x;
}
void TrainManager::printdate(int x) {
    print(x/100);defaultOut << '-';print(x%100);
}
void TrainManager::printtime(int x) {
    print(x/100);defaultOut << ':';print(x%100);
}
