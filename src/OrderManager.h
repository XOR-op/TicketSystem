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

        DiskLoc_T extend(const order* record, DiskLoc_T where);

    public:
        /*
         * ptr should be guaranteed to be big enough
         *
         * @return: size of the block
         * @modify: where will be pointed to next offset
         */
        int getRecord(DiskLoc_T* where, order* ptr);

        /*
         * if offset_val is not null, where it points will become the offset in the block
         *
         * @return: current head offset
         */
        std::pair<DiskLoc_T,int> appendRecord(DiskLoc_T where, const order* record,int* offset_val= nullptr);

        /*
         * @return: current head offset
         */
        DiskLoc_T createRecord();

        /*
         * do not check permission
         * called by UserManager and not directly called by logic codes
         */
        void printAllOrders(std::ostream& ofs, DiskLoc_T head);

        /*
         * @return: whether the operation succeeds
         */
        std::pair<bool,order> refundOrder(DiskLoc_T head, int n);

        void setSuccess(DiskLoc_T block,int offset_in_block);

        explicit OrderManager(const std::string& file_path);

        ~OrderManager();

        static void Init(const std::string& path) {
            std::fstream file(path, ios::binary | ios::out);
            DiskLoc_T sz = sizeof(DiskLoc_T);
            file.write((char*) &sz, sizeof(sz));
            file.close();
        }
    };
}
#endif //TICKETSYSTEM_ORDERMANAGER_H
