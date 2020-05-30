//
// Created by vortox on 30/5/20.
//

#ifndef TICKETSYSTEM_PENDINGTICKETMANAGER_H
#define TICKETSYSTEM_PENDINGTICKETMANAGER_H
#include "structure.h"
#include "UserOrderManager.h"
#include "../basic_component/LRUBPtree.h"
namespace t_sys{
    class PendingTicketManager{
    private:
        std::fstream pendingFile;
        DiskLoc_T pending_file_size;
        cache::LRUCache<DiskLoc_T ,pending_order> pending_cache;
    public:
        explicit PendingTicketManager(const std::string& path);
        ~PendingTicketManager();
        void allocate_tickets(UserOrderManager* ord_manager, train* ptr, const order* Order);
        void add_pendingorder(pending_order* record, train* tra);
        void cancel_pending(int order_key,train* train);
        static void Init(const std::string& path){init_subprocess(path);}
    };
}
#endif //TICKETSYSTEM_PENDINGTICKETMANAGER_H
