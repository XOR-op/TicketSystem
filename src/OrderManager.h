//
// Created by vortox on 29/3/20.
//

#ifndef TICKETSYSTEM_ORDERMANAGER_H
#define TICKETSYSTEM_ORDERMANAGER_H

#include <fstream>
#include <cstring>
#include <iostream>
#include "structure.h"
#include "../bptree/cache.h"
using std::ios;
namespace t_sys{
    /*
     * designed for order records
     * file structure:  BLOCK1 <- BLOCK2 <- BLOCK_HEAD
     * Block structure: nextOffset(DiskLoc_T)|size(int)|data
     */
    struct record_block{
        const static int count=20;
        int size;

    };
    class OrderManager{
    private:
        const static int DATA_SIZE= sizeof(order);
        const static int count=20;
        std::fstream file;
        DiskLoc_T file_size;
        DiskLoc_T extend(const order* record,DiskLoc_T where){
            // construct buffer
            char buffer[sizeof(DiskLoc_T)+sizeof(int)+DATA_SIZE];
            char* buf=buffer;
            int size=(record!= nullptr);
# define write_attribute(ATTR) do{memcpy(buf,(void*)&ATTR,sizeof(ATTR));buf+=sizeof(ATTR);}while(0)
            write_attribute(where);
            write_attribute(size);
            if(record) {
                write_attribute(*record);
            }
#undef write_attribute
            // write buffer
            file.seekp(file_size);
            file.write(buffer, sizeof(buffer));
            where=file_size;
            file_size+=sizeof(DiskLoc_T)+sizeof(int)+DATA_SIZE*count;
            return where;
        }
        void initialize(const std::string& path){
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
    public:
        /*
         * ptr should be guaranteed to be big enough
         *
         * @return: size of the block
         * @modify: where will be pointed to next offset
         */
        int getRecord(DiskLoc_T* where, order* ptr){
            int size;
            file.seekg(*where);
            file.read((char*)where, sizeof(DiskLoc_T));
            file.read((char*)&size, sizeof(int));
            file.read((char*)ptr,DATA_SIZE*size);
            return size;
        }
        DiskLoc_T appendRecord(DiskLoc_T where,const order* record){
            int size;
            file.seekg(where+sizeof(DiskLoc_T));
            file.read((char*)&size, sizeof(int));
            if(size==count){
                // extend
                return extend(record,where);
            } else {
                file.seekp(where+sizeof(int)+sizeof(DiskLoc_T)+DATA_SIZE*size);
                file.write((char*) record, DATA_SIZE);
                return where;
            }
        }
        DiskLoc_T createRecord(){
//            return 0; // **FOR TEST ONLY**
            return extend(nullptr,0);
        }
        explicit OrderManager(const std::string& file_path, bool create_flag= false){
            if(create_flag)initialize(file_path);
            file.open(file_path);
            // read metadata
            char buf[sizeof(file_size)];
            char* ptr = buf;
            file.seekg(0);
            file.read(buf, sizeof(buf));
#define read_attribute(ATTR) memcpy((void*)&ATTR,ptr,sizeof(ATTR));ptr+=sizeof(ATTR)
            read_attribute(file_size);
#undef read_attribute
        }
        ~OrderManager(){
            char buf[sizeof(file_size)];
            char* ptr = buf;
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
            write_attribute(file_size);
#undef write_attribute
            file.seekg(0);
            file.write(buf, sizeof(buf));
            file.close();
        }
//        OrderManager()=default; // **FOR TEST ONLY**
    };
}
#endif //TICKETSYSTEM_ORDERMANAGER_H
