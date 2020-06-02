//
// Created by vortox on 30/5/20.
//

#ifndef TICKETSYSTEM_DEBUG_H
#define TICKETSYSTEM_DEBUG_H
//#include <unistd.h>
#include <fstream>
#include <iostream>
//#define NDEBUG
namespace Debug{
    /*
    static void process_mem_usage(double& vm_usage, double& resident_set)
    {
         //
         // Source code from
         // https://stackoverflow.com/questions/669438/how-to-get-memory-usage-at-runtime-using-c
         //
        using std::ios_base;
        using std::ifstream;
        using std::string;

        vm_usage     = 0.0;
        resident_set = 0.0;

        // 'file' stat seems to give the most reliable results
        //
        ifstream stat_stream("/proc/self/stat",ios_base::in);

        // dummy vars for leading entries in stat that we don't care about
        //
        string pid, comm, state, ppid, pgrp, session, tty_nr;
        string tpgid, flags, minflt, cminflt, majflt, cmajflt;
        string utime, stime, cutime, cstime, priority, nice;
        string O, itrealvalue, starttime;

        // the two fields we want
        //
        unsigned long vsize;
        long rss;

        stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
                    >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
                    >> utime >> stime >> cutime >> cstime >> priority >> nice
                    >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

        stat_stream.close();

        long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
        vm_usage     = vsize / 1024.0;
        resident_set = rss * page_size_kb;
    }
    */
    class CacheMissRater{
        std::ostream& printPortion(int a,int b){
            if(total_count==0){
                std::cerr<<"May invoke wrong"<<std::endl;
                return std::cerr;
            }
            return std::cout<<(100*(double)a/(double )b)<<'%'<<std::endl;
        }
    public:
        int hit_count=0,
            hot_hit_count=0,
            cold_hit_count=0,
            miss_count=0,
            total_count=0,
            dirty_count=0;
        void hot(){
            ++hit_count,++hot_hit_count,++total_count;
        }
        void cold(){
            ++hit_count,++cold_hit_count,++total_count;
        }
        void miss(){
            ++miss_count,++total_count;
        }
        void hit(){
            ++hit_count,++total_count;
        }
        void dirty(){
            ++dirty_count;
        }
        bool good()const {return ((hot_hit_count+cold_hit_count==hit_count)||(hot_hit_count==0&&cold_hit_count==0))&&
                    (hit_count+miss_count==total_count);}
        void hitRate(){
            std::cout<<"Hit rate:";
            printPortion(hit_count,total_count);
        }
        void missRate(){
            std::cout<<"Miss rate:";
            printPortion(miss_count,total_count);
        }
//        void hotRate(){printPortion(hot_hit_count,total_count);}
//        void coldRate(){printPortion(miss_count,total_count);}
        void hotByHit(){
            std::cout<<"Hot rate:";
            printPortion(hot_hit_count,hit_count);
        }
        void coldByHit(){
            std::cout<<"Cold rate:";
            printPortion(miss_count,hit_count);
        }
    };
    extern CacheMissRater LRUrater,SLRUrater;
}
#endif //TICKETSYSTEM_DEBUG_H
