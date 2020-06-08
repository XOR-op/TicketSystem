//
// Created by vortox on 29/3/20.
//

#ifndef TICKETSYSTEM_USERORDERMANAGER_H
#define TICKETSYSTEM_USERORDERMANAGER_H

#include <fstream>
#include <cstring>
#include <iostream>
#include "structure.h"
#include "../basic_component/LRU.h"
#include "../basic_component/SLRU.h"
#include "../basic_component/PageManager.h"

using std::ios;
namespace t_sys {
    /*
     * designed for order records
     * file structure:  BLOCK1 <- BLOCK2 <- BLOCK_HEAD
     * Block structure: nextOffset(DiskLoc_T)|size(int)|data
     */
    struct _order_block{
        const static int COUNT = 20;
        DiskLoc_T nextOffset;
        int size;
        order data[COUNT];
    };

    class UserOrderManager {
    private:
        const static int DATA_SIZE = sizeof(order);
        const static DiskLoc_T NO_NEXT = 0;
//        std::fstream file;
        std::fstream file;
        cache::SLRUCache<DiskLoc_T,_order_block> order_block_cache;
        DiskLoc_T order_file_size;

        DiskLoc_T extend(const order* record, DiskLoc_T where);

        static void readBlock(std::fstream& ifs,DiskLoc_T where,_order_block* ptr);
        static void writeBlock(std::fstream& ofs,DiskLoc_T where,const _order_block* ptr);

    public:
        /*
         * warning: returned pointer may fail after several operations
         */
        _order_block* getRecord(DiskLoc_T where);

        /*
         * if offset_val is not null, where it points will become the offset in the block
         *
         * @return: current head offset
         */
        std::pair<DiskLoc_T,int> appendRecord(DiskLoc_T where, const order* record);

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

        explicit UserOrderManager(const std::string& file_path);

        ~UserOrderManager();

        static void Init(const std::string& path);
    };
}
#endif //TICKETSYSTEM_USERORDERMANAGER_H
