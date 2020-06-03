#include "TrainManager.h"
#include "../include/SerializationHelper.h"

using namespace t_sys;
using std::pair;
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
    int hh = end%100-start%100;
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
int TrainManager::findtrainID(const trainID_t& t) {
    auto rt = trainidToOffset.search(t);
    if (rt.second)return 1;
    else return 0;
}
void TrainManager::print(int x) {
    if (x < 10)defaultOut << '0' << x; else defaultOut << x;
}
void TrainManager::printdate(int x) {
    print(x/100);
    defaultOut << '-';
    print(x%100);
}
void TrainManager::printtime(int x) {
    print(x/100);
    defaultOut << ':';
    print(x%100);
}
int getDate(const int x) {
    return x/10000;
}

int getTime(const int x) {
    return x%10000;
}
int station_id_f(int x) { return x/10000; }
int train_id_f(int x) { return x%10000; }
static void loadTrain(std::fstream& ifs, DiskLoc_T offset, train* tra) {
    ifs.seekg(offset);
    ifs.read((char*) tra, sizeof(train));
}
static void saveTrain(std::fstream& ofs, DiskLoc_T offset, const train* tra) {
    ofs.seekp(offset);
    ofs.write((char*) tra, sizeof(train));
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
    // todo fix
    /*
    auto* new_node = new freenode;
    new_node->nxt = head;
    new_node->pos = loc;
    head = new_node;
     */
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
        //std::cout<<i<<' '<<stationlist[station_t(train_ptr->stations[1])]<<endl;
        assert(stationlist[the_station]);
        stationTotrain.insert(stationlist[the_station]*10000+train_num, i);
    }
    train_num++;
    int date = 0, tim = train_ptr->startTime;
    for (int i = 0; i < train_ptr->stationNum; i++) {
        int tmp = train_ptr->travelTimes[i];
        train_ptr->travelTimes[i] = (i == 0) ? 0 : date*10000+tim;
        if (i != 0 && i != (train_ptr->stationNum)-1)
            addtime(date, tim, train_ptr->stopoverTimes[i]);
        train_ptr->stopoverTimes[i] = (i == (train_ptr->stationNum)-1) ? 0 : date*10000+tim;
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
void TrainManager::print_ticket(const pair<int, pair<int, int>>& A, int date) {
    trainID_t tra = trainlist[A.first];
    DiskLoc_T loc = trainidToOffset.search(tra).first;
    auto* train_ptr = train_cache.get(loc);
    int s = A.second.first/100, t = A.second.first%100;
    int days = getDate(train_ptr->stopoverTimes[s]);
    int startday = calcstartday(date, days);
    int start = (train_ptr->saleDate)/10000;
    int end = (train_ptr->saleDate)%10000;
    defaultOut << (train_ptr->trainID.ID) << ' ' << train_ptr->stations[s]<< ' ';
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
    ds::vector<pair<int, pair<int, int>>> Ans;  // {train_id,{k1th_train*100+k2th_train,key}}


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
                    int key = (order == 0) ?
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
    static auto cmp = [&Ans](int x, int y) -> bool { return Ans[x].second.second < Ans[y].second.second; };
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
    int minkey = INT32_MAX;
    pair<int, pair<int, int>> A, B; // {train,{stat_s*100+stat_t,date}}
    for (int i = 0; i < S.size(); i++)
        for (int j = 0; j < T.size(); j++)
            if (train_id_f(S[i].first) != train_id_f(T[j].first)) {
                trainID_t t1 = trainlist[train_id_f(S[i].first)], t2 = trainlist[train_id_f(T[j].first)];
                DiskLoc_T loc = trainidToOffset.search(t1).first, loc2 = trainidToOffset.search(t2).first;
                auto* train_ptr_1st = train_cache.get(loc);
                auto* train_ptr_2nd = train_cache.get(loc2);
                int days = getDate(train_ptr_1st->stopoverTimes[S[i].second]);
                int startday = calcstartday(date, days);
                int first_train_start_date = (train_ptr_1st->saleDate)/10000;
                int first_train_end_date = (train_ptr_1st->saleDate)%10000;
                int second_train_start_date = (train_ptr_2nd->saleDate)/10000;
                int second_train_end_date = (train_ptr_2nd->saleDate)%10000;
                // check s_train date
                if (startday < first_train_start_date || startday > first_train_end_date)continue;
                //check_transfer
                int first_kth_station = S[i].second, second_kth_station = T[j].second;
                for (int first_train_mid_station = first_kth_station+1;
                     first_train_mid_station < train_ptr_1st->stationNum; first_train_mid_station++) {
                    for (int second_train_mid_station = 0;
                         second_train_mid_station < second_kth_station; second_train_mid_station++)
                        if (strcmp(train_ptr_1st->stations[first_train_mid_station],
                                   train_ptr_2nd->stations[second_train_mid_station]) == 0) {
                            // T_arrive <= T_leave
                            int stopoverday = 0;
                            if (getTime(train_ptr_1st->travelTimes[first_train_mid_station]) >
                                getTime(train_ptr_2nd->stopoverTimes[second_train_mid_station]))
                                stopoverday = 1;
                            // check t_train date
                            int second_leave_day = startday, ___ = 0;
                            addtime(second_leave_day, ___,
                                    24*60*(getDate(train_ptr_1st->travelTimes[first_train_mid_station])+stopoverday));
                            int days2 = getDate(train_ptr_2nd->stopoverTimes[second_train_mid_station]);
                            int beststartday2 = calcstartday(second_leave_day, days2); // 2th train earliest leave day
                            if (beststartday2 > second_train_end_date)break;
                            int interval_day = stopoverday;
                            if (beststartday2 < second_train_start_date) {
                                // wait until second train can leave
                                interval_day = second_train_start_date-beststartday2+stopoverday;
                                second_leave_day = second_train_start_date;
                                addtime(second_leave_day, ___,
                                        24*60*getDate(train_ptr_2nd->stopoverTimes[second_train_mid_station]));
                            }
                            int key;
                            if (order == 0) {
                                key = calctime(train_ptr_1st->stopoverTimes[first_kth_station],
                                               train_ptr_1st->travelTimes[first_train_mid_station])+
                                      calctime(getTime(train_ptr_1st->travelTimes[first_train_mid_station]),
                                               10000*interval_day+
                                               train_ptr_2nd->stopoverTimes[second_train_mid_station])+
                                      calctime(train_ptr_2nd->stopoverTimes[second_train_mid_station],
                                               train_ptr_2nd->travelTimes[second_kth_station]);
                            } else {
                                key = train_ptr_1st->prices[first_train_mid_station]-
                                      train_ptr_1st->prices[first_kth_station]+
                                      train_ptr_2nd->prices[second_kth_station]-
                                      train_ptr_2nd->prices[second_train_mid_station];
                            }
                            if (key < minkey) {
                                minkey = key;
                                assert(strcmp(train_ptr_1st->stations[first_train_mid_station],
                                              train_ptr_2nd->stations[second_train_mid_station]) == 0);
                                A = std::make_pair((int) train_id_f(S[i].first),
                                                   std::make_pair(first_kth_station*100+first_train_mid_station, date));
                                B = std::make_pair((int) train_id_f(T[j].first),
                                                   std::make_pair(second_train_mid_station*100+second_kth_station,
                                                                  second_leave_day));
                            }
                            break;
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
        pend_manager->cancel_pending(Order.key, ptr);
    }
    train_cache.set_dirty_bit(loc);
    return true;
}
TrainManager::TrainManager(const std::string& file_path, const std::string& trainid_index_path,
                           const std::string& station_index_path, const std::string& train_info_path,
                           const std::string& station_info_path)
        : train_cache(51, [this](DiskLoc_T off, train* tra) {
                          assert(trainFile.good());
                          loadTrain(trainFile, off, tra);
                      },
                      [this](DiskLoc_T off, const train* tra) {
                          assert(trainFile.good());
                          saveTrain(trainFile, off, tra);
                      }),
          trainidToOffset(trainid_index_path, 107),
          stationTotrain(station_index_path, 157),
          head(nullptr), defaultOut(std::cout), train_info_path(train_info_path), station_info_path(station_info_path) {
    // todo fix head because it doesn't work on file yet.
    trainFile.open(file_path);
    //metadata
    char buf[sizeof(train_file_size)+sizeof(int)*3+sizeof(head)];
    char* ptr = buf;
    trainFile.seekg(0);
    trainFile.read(buf, sizeof(buf));
#define read_attribute(ATTR) memcpy((void*)&ATTR,ptr,sizeof(ATTR));ptr+=sizeof(ATTR)
    read_attribute(train_file_size);
    read_attribute(train_num);
    read_attribute(station_num);
    read_attribute(ticket_num);
    read_attribute(head);
#undef read_attribute
    ds::SerialVector<trainID_t>::deserialize(trainlist, train_info_path);
    ds::SerialMap<station_t, int>::deserialize(stationlist, station_info_path);
}

TrainManager::~TrainManager() {
    // write metadata
    char buf[sizeof(train_file_size)+sizeof(int)*3+sizeof(head)];
    char* ptr = buf;
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
    write_attribute(train_file_size);
    write_attribute(train_num);
    write_attribute(station_num);
    write_attribute(ticket_num);
    write_attribute(head);
#undef write_attribute
    assert(trainFile.good());
    trainFile.seekp(0);
    trainFile.write(buf, sizeof(buf));
    train_cache.destruct();
    trainFile.close();
    ds::SerialVector<trainID_t>::serialize(trainlist, train_info_path);
    ds::SerialMap<station_t, int>::serialize(stationlist, station_info_path);
}


void TrainManager::Init(const std::string& file_path) {
    std::fstream f(file_path, ios::out | ios::binary);
    char buf[sizeof(DiskLoc_T)+sizeof(int)*3+sizeof(freenode*)];
    char* ptr = buf;
    DiskLoc_T sz = sizeof(buf);
    int train_num = 0, station_num = 0, ticket_num = 0;
    freenode* p = nullptr;
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
    write_attribute(sz);
    write_attribute(train_num);
    write_attribute(station_num);
    write_attribute(ticket_num);
    write_attribute(p);
#undef write_attribute
    f.write(buf, sizeof(buf));
    f.close();

}
