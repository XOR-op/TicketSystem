//
// Created by vortox on 28/4/20.
//

#ifndef TICKETSYSTEM_PAGEMANAGER_H
#define TICKETSYSTEM_PAGEMANAGER_H

#include <memory>
#include <cstring>
#include <cstdio>
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

    void PageManager::read(char* dst, DiskLoc_T offset, size_t size) {
        DiskLoc_T p_offset = page_offset(offset);
        char* ptr = (char*) cache.get(p_offset);
        auto first_size = p_offset+PAGE_SIZE-offset;
        if (size <= first_size) {
            memcpy(dst, ptr+offset-p_offset, size);
        } else {
            // read many pages
            memcpy(dst, ptr+offset-p_offset, first_size);
            for (dst += first_size, size -= first_size, p_offset += PAGE_SIZE;
                 size >= PAGE_SIZE; dst += PAGE_SIZE, size -= PAGE_SIZE, p_offset += PAGE_SIZE) {
                ptr = (char*) cache.get(p_offset);
                memcpy(dst, ptr, PAGE_SIZE);
            }
            if (size > 0) {
                ptr = (char*) cache.get(p_offset);
                memcpy(dst, ptr, size);
            }
        }
    }
    void PageManager::write(const char* src, DiskLoc_T offset, size_t size) {
        if (offset+size >= file_size) {
            // pseudo ftruncate()
            file_size = page_offset(offset+size)+PAGE_SIZE;
            char null_char = '\0';
            fseek(file, file_size, SEEK_SET);
            fwrite(&null_char, 1, 1, file);
        }
        DiskLoc_T p_offset = page_offset(offset);
        char* ptr = (char*) cache.get(p_offset);
        cache.dirty_bit_set(p_offset);
        auto first_size = p_offset+PAGE_SIZE-offset;
        if (size <= first_size) {
            memcpy(ptr+offset-p_offset, src, size);
        } else {
            // write many pages
            memcpy(ptr+offset-p_offset, src, first_size);
            for (src += first_size, size -= first_size, p_offset += PAGE_SIZE;
                 size >= PAGE_SIZE; src += PAGE_SIZE, size -= PAGE_SIZE, p_offset += PAGE_SIZE) {
                ptr = (char*) cache.get(p_offset);
                cache.dirty_bit_set(p_offset);
                memcpy(ptr, src, PAGE_SIZE);
            }
            if (size > 0) {
                ptr = (char*) cache.get(p_offset);
                cache.dirty_bit_set(p_offset);
                memcpy(ptr, src, size);
            }
        }


    }
    PageManager::PageManager(const std::string& path) : cache(PAGE_CNT,
                                                              [this](DiskLoc_T offset, Page* p) {
                                                                  readPage(this->file, offset, p);
                                                              },
                                                              [this](DiskLoc_T offset, const Page* p) {
                                                                  writePage(this->file, offset, p);
                                                              }) {
        file = fopen(path.c_str(), "rb+");
        fseek(file, 0, SEEK_END);
        file_size=ftell(file);
        rewind(file);
    }
    PageManager::~PageManager() {
        cache.destruct();
        fclose(file);
    }

}
#endif //TICKETSYSTEM_PAGEMANAGER_H
