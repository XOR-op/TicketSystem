//
// Created by vortox on 28/4/20.
//

#ifndef TICKETSYSTEM_PAGEMANAGER_H
#define TICKETSYSTEM_PAGEMANAGER_H

#include <memory>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "../bptree/cache.h"
#include "global.h"

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
        int fd;
        static DiskLoc_T page_offset(DiskLoc_T offset) {
            return offset-offset%PAGE_SIZE;
        }
        static void readPage(int fd,DiskLoc_T offset,Page* p){
            lseek(fd,offset,SEEK_SET);
            ::read(fd,p,PAGE_SIZE);
        }
        static void writePage(int fd,DiskLoc_T offset,const Page* p){
            lseek(fd,offset,SEEK_SET);
            ::write(fd,p,PAGE_SIZE);
        }
    public:
        void read(char* dst, DiskLoc_T offset, size_t size);
        void write(const char* src, DiskLoc_T offset, size_t size);
        /*
         * This function MUST BE CALLED before any WRITE, or undefined behavior will happen
         */
        void setSize(size_t size) {
            file_size = size;
        }
        explicit PageManager(const std::string& path);
        ~PageManager();
    };

    void PageManager::read(char* dst, DiskLoc_T offset, size_t size) {
        DiskLoc_T p_offset = page_offset(offset);
        Page* ptr = cache.get(p_offset);
        auto first_size = p_offset+PAGE_SIZE-offset;
        if (size <= first_size) {
            memcpy(dst, ptr+offset-p_offset, size);
        } else {
            memcpy(dst, ptr+offset-p_offset, first_size);
            for (dst += first_size, size -= first_size, p_offset += first_size;
                 size >= PAGE_SIZE; dst += PAGE_SIZE, size -= PAGE_SIZE, p_offset += PAGE_SIZE) {
                ptr = cache.get(p_offset);
                memcpy(dst, ptr, PAGE_SIZE);
            }
            if (size > 0) {
                ptr = cache.get(p_offset);
                memcpy(dst, ptr, size);
            }
        }
    }
    void PageManager::write(const char* src, DiskLoc_T offset, size_t size) {
        if (offset+size >= file_size) {
            file_size = page_offset(offset+size)+PAGE_SIZE;
            ftruncate(fd, file_size);
        }
        DiskLoc_T p_offset = page_offset(offset);
        Page* ptr = cache.get(p_offset);
        cache.dirty_bit_set(p_offset);
        auto first_size = p_offset+PAGE_SIZE-offset;
        if (size <= first_size) {
            memcpy(ptr+offset-p_offset, src, size);
        } else {
            memcpy(ptr+offset-p_offset, src, first_size);
            for (src += first_size, size -= first_size, p_offset += first_size;
                 size >= PAGE_SIZE; src += PAGE_SIZE, size -= PAGE_SIZE, p_offset += PAGE_SIZE) {
                ptr = cache.get(p_offset);
                cache.dirty_bit_set(p_offset);
                memcpy(ptr, src, PAGE_SIZE);
            }
            if (size > 0) {
                ptr = cache.get(p_offset);
                cache.dirty_bit_set(p_offset);
                memcpy(ptr, src, size);
            }
        }


    }
    PageManager::PageManager(const std::string& path):cache(PAGE_CNT,
            [this](DiskLoc_T offset,Page* p){readPage(this->fd,offset,p);},
            [this](DiskLoc_T offset,const Page* p){writePage(this->fd,offset,p);}) {
        file_size=PAGE_SIZE;
        fd=open(path.c_str(),O_RDWR,O_CREAT);
    }
    PageManager::~PageManager() {
        cache.destruct();
        close(fd);
    }

}
#endif //TICKETSYSTEM_PAGEMANAGER_H
