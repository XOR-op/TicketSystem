//
// Created by vortox on 28/4/20.
//

#ifndef TICKETSYSTEM_PAGEMANAGER_H
#define TICKETSYSTEM_PAGEMANAGER_H
#include "../bptree/cache.h"
#include "global.h"
namespace t_sys{

    const static int PAGE_SIZE=4096;
    struct Page{
        char data[PAGE_SIZE];
    };
    class PageManager{
    private:
        const static int PAGE_CNT=256;
        cache::LRUCache<DiskLoc_T,Page> cache;
    public:
        void read(char* dst,DiskLoc_T offset,size_t size);
        void write(const char* src,DiskLoc_T offset,size_t size);
        PageManager(const std::string& path);
        ~PageManager();
    };
}
#endif //TICKETSYSTEM_PAGEMANAGER_H
