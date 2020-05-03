//
// Created by vortox on 28/4/20.
//

#ifndef TICKETSYSTEM_PAGEMANAGER_H
#define TICKETSYSTEM_PAGEMANAGER_H

#include <memory>
#include <cstring>
#include <cstdio>
#include "cache.h"
#include "../src/global.h"

using std::size_t;
namespace t_sys {

    const static int PAGE_SIZE = 4096;
    struct Page {
        char data[PAGE_SIZE];
    };

    class PageManager {
    private:
        const static int PAGE_CNT = 256;
        cache::LRUCache<DiskLoc_T, Page> cache;
        size_t file_size;
        FILE* file;
        static DiskLoc_T page_offset(DiskLoc_T offset) {
            return offset-offset%PAGE_SIZE;
        }
        static void readPage(FILE* fd, DiskLoc_T offset, Page* p) {
            fseek(fd, offset, SEEK_SET);
            fread(p, 1, PAGE_SIZE, fd);
        }
        static void writePage(FILE* fd, DiskLoc_T offset, const Page* p) {
            fseek(fd, offset, SEEK_SET);
            fwrite(p, 1, PAGE_SIZE, fd);
        }
    public:
        void read(char* dst, DiskLoc_T offset, size_t size);
        void write(const char* src, DiskLoc_T offset, size_t size);
        explicit PageManager(const std::string& path);
        ~PageManager();
    };


}
#endif //TICKETSYSTEM_PAGEMANAGER_H
