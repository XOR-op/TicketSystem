//
// Created by vortox on 30/5/20.
//

#ifndef TICKETSYSTEM_PENDINGTICKETMANAGER_H
#define TICKETSYSTEM_PENDINGTICKETMANAGER_H
#include "structure.h"
#include "UserOrderManager.h"
#include "../basic_component/LRUBPtree.h"
#include "../basic_component/PageManager.h"
namespace t_sys{
    class PendingTicketManager{
    private:
        DiskLoc_T pending_file_size;
        const static DiskLoc_T NONE=0;
//        std::fstream pendingFile;
//        cache::SLRUCache<DiskLoc_T ,pending_order> pending_cache;
        DiskLoc_T freelist_head;
        PageManager pendingFile;
        void add_free_block(DiskLoc_T where);
    public:
        explicit PendingTicketManager(const std::string& path);
        ~PendingTicketManager();
        void allocate_tickets(UserOrderManager* ord_manager, train* train_ptr, const order* Order);
        void add_pendingorder(pending_order* record, train* tra);
        void cancel_pending(int order_key,train* train);
        static void Init(const std::string& path);
    };
}
#endif //TICKETSYSTEM_PENDINGTICKETMANAGER_H
