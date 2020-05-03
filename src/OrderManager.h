//
// Created by vortox on 29/3/20.
//

#ifndef TICKETSYSTEM_ORDERMANAGER_H
#define TICKETSYSTEM_ORDERMANAGER_H

#include <fstream>
#include <cstring>
#include <iostream>
#include "structure.h"
#include "../basic_component//cache.h"
#include "../basic_component/PageManager.h"

using std::ios;
namespace t_sys {
    /*
     * designed for order records
     * file structure:  BLOCK1 <- BLOCK2 <- BLOCK_HEAD
     * Block structure: nextOffset(DiskLoc_T)|size(int)|data
     */
    struct record_block {
        const static int count = 20;
        int size;

    };

    class OrderManager {
    private:
        const static int DATA_SIZE = sizeof(order);
        const static int count = 20;
        const static DiskLoc_T NO_NEXT = 0;
//        std::fstream file;
        PageManager file;
        DiskLoc_T file_size;

        DiskLoc_T extend(const order* record, DiskLoc_T where) {
            // construct buffer
            char buffer[sizeof(DiskLoc_T)+sizeof(int)+DATA_SIZE];
            char* buf = buffer;
            int size = (record != nullptr);
# define write_attribute(ATTR) do{memcpy(buf,(void*)&ATTR,sizeof(ATTR));buf+=sizeof(ATTR);}while(0)
            write_attribute(where);
            write_attribute(size);
            if (record) {
                write_attribute(*record);
            }
#undef write_attribute
            // write buffer
            file.write(buffer, file_size, sizeof(buffer));
            where = file_size;
            file_size += sizeof(DiskLoc_T)+sizeof(int)+DATA_SIZE*count;
            return where;
        }

        void initialize() {
            char buf[sizeof(DiskLoc_T)];
            char* ptr = buf;
            DiskLoc_T sz = sizeof(DiskLoc_T);
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
            write_attribute(sz);
#undef write_attribute
            file.write(buf, 0, sizeof(buf));
        }
    public:
        /*
         * ptr should be guaranteed to be big enough
         *
         * @return: size of the block
         * @modify: where will be pointed to next offset
         */
        int getRecord(DiskLoc_T* where, order* ptr);
        DiskLoc_T appendRecord(DiskLoc_T where, const order* record);
        DiskLoc_T createRecord();
        /*
         * do not check permission
         * called by UserManager and not directly called by logic codes
         */
        void printAllOrders(std::ostream& ofs, DiskLoc_T head);
        explicit OrderManager(const std::string& file_path, bool create_flag = false);
        ~OrderManager();
    };
}
#endif //TICKETSYSTEM_ORDERMANAGER_H
