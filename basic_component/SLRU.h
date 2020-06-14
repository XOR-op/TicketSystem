//
// Created by vortox on 30/5/20.
//

#ifndef TICKETSYSTEM_SLRU_H
#define TICKETSYSTEM_SLRU_H
#include <functional>
#include <cassert>
#include "../include/unordered_map.h"
#include "../include/debug.h"
//extern Debug::CacheMissRater SLRUrater;
//using Debug::SLRUrater;
namespace cache{
    template <typename DiskLoc_T,typename T>
    using func_expire_t =std::function<void(DiskLoc_T,T*)>;
    template <typename DiskLoc_T,typename T>
    using func_load_t=std::function<void(DiskLoc_T, T*)>;

    template<typename DiskLoc_T,typename T>
    struct _node_{
        _node_ *prev,*next;
        DiskLoc_T offset;
        bool dirty_bit;
        union {T data;}; // avoid construction
        void detach(){
            next->prev=prev;
            prev->next=next;
        }
        void attach(_node_* ptr){
            prev=ptr,next=ptr->next;
            next->prev= this,prev->next= this;
        }
//        _node_(){dirty_bit= false;}
//        ~_node_(){
//            data.~T();
//        }
    };

    template <typename DiskLoc_T,typename T>
    class SLRUCache {
    private:
        typedef T* DataPtr;
        typedef _node_<DiskLoc_T,T>* node_ptr;
        ds::unordered_map<DiskLoc_T,node_ptr> hot_table;
        ds::unordered_map<DiskLoc_T,node_ptr> cold_table;
        node_ptr hot_head,cold_head;
        size_t hot_max,cold_max;
        node_ptr memory_pool;
        node_ptr free_list_ptr;
        node_ptr block_new(){
            auto rt=free_list_ptr;
//            free_list_ptr=(node_ptr)*((std::size_t*)free_list_ptr);
            free_list_ptr=free_list_ptr->next;
            rt->dirty_bit=false;
            return rt;
        }

        void block_delete(node_ptr ptr){
            ptr->data.~T();
            ptr->next=free_list_ptr;
            free_list_ptr=ptr;
        }

        func_load_t<DiskLoc_T,T> f_load;
        func_expire_t<DiskLoc_T,T> f_expire;
        void expire(node_ptr np) {
            if(np->dirty_bit){
                f_expire(np->offset,&np->data);
#ifndef NDEBUG
                SLRUrater.dirty();
#endif
            }
            block_delete(np);
        }
        node_ptr load(DiskLoc_T offset){
//            auto ptr=new _node_<DiskLoc_T,T>;
            auto ptr=block_new();
            f_load(offset,&ptr->data);
            ptr->offset=offset;
            return ptr;
        }
        void expireAll(node_ptr& head){
            node_ptr cur=head->next,bk;
            while (cur!=head){
                bk=cur->next;
                expire(cur);
                cur=bk;
            }
            free(head);
            head= nullptr;
        }
    public:
        /*
         * load function is used to read a structure from offset(DiskLoc_T) in disk to space(T*) in memory
         * expire function is used to write a structure to offset(DiskLoc_T) in disk from space(T*) in memory
         */

        SLRUCache(size_t size,func_load_t<DiskLoc_T,T> load_func, func_expire_t<DiskLoc_T,T> expire_func,double ratio=0.625)
                :  f_load(load_func), f_expire(expire_func){
            hot_head=(node_ptr)malloc(sizeof(_node_<DiskLoc_T,T>));
            cold_head=(node_ptr )malloc(sizeof(_node_<DiskLoc_T,T>));
            hot_head->next=hot_head->prev=hot_head;
            cold_head->prev=cold_head->next=cold_head;
            hot_max=size*ratio;
            cold_max=size-hot_max;
            memory_pool=(node_ptr)malloc(sizeof(_node_<DiskLoc_T,T>)*(size));
            for(int i=1;i<size;++i)memory_pool[i-1].next=memory_pool+i;
            free_list_ptr=memory_pool;
        }
#ifndef NDEBUG
        Debug::CacheMissRater SLRUrater;
#endif
        SLRUCache(const SLRUCache&) = delete;

        SLRUCache& operator=(const SLRUCache&) = delete;

        bool remove(DiskLoc_T offset) {
            if(auto iter=hot_table.find(offset);iter!=hot_table.end()){
                node_ptr dying=iter->second;
                dying->detach();
                hot_table.erase(dying->offset);
                expire(dying);
                return true;
            } else if(iter=cold_table.find(offset);iter!=cold_table.end()){
                node_ptr dying=iter->second;
                dying->detach();
                cold_table.erase(dying->offset);
                expire(dying);
                return true;
            } else return false;
        }

        DataPtr get(DiskLoc_T offset) {
            if(auto iter=hot_table.find(offset);iter!=hot_table.end()){
#ifndef NDEBUG
                SLRUrater.hot();
#endif
                node_ptr cur=iter->second;
                cur->detach();
                cur->attach(hot_head);
                return &(iter->second->data);
            } else if(iter=cold_table.find(offset);iter!=cold_table.end()){
#ifndef NDEBUG
                SLRUrater.cold();
#endif
                node_ptr cur=iter->second;
                cur->detach();
                cold_table.erase(offset);
                cur->attach(hot_head);
                hot_table[offset]=cur;
                if(hot_table.size()>=hot_max){
                    // hot overflow
                    if(cold_table.size()>=cold_max){
                        node_ptr dying=cold_head->prev;
                        cold_table.erase(dying->offset);
                        dying->detach();
                        expire(dying);
                    }
                    node_ptr move_out=hot_head->prev;
                    move_out->detach();
                    hot_table.erase(move_out->offset);
                    move_out->attach(cold_head);
                    cold_table[move_out->offset]=move_out;
                }
                return &(cur->data);
            } else{
                // not in cache
#ifndef NDEBUG
                SLRUrater.miss();
#endif
                if(cold_table.size()>=cold_max){
                    node_ptr dying=cold_head->prev;
                    cold_table.erase(dying->offset);
                    dying->detach();
                    expire(dying);
                }
                node_ptr cur=load(offset);
                cur->attach(cold_head);
                cold_table[offset]=cur;
                return &(cur->data);
            }
        }
        // must be called manually after write
        // or you will lose your write !!!
        void set_dirty_bit(DiskLoc_T offset){
            if(auto iter=hot_table.find(offset);iter!=hot_table.end()){
                iter->second->dirty_bit=true;
                return;
            }else if(iter=cold_table.find(offset);iter!=cold_table.end()){
                iter->second->dirty_bit=true;
                return;
            }
            assert(0);
        }
        void destruct(){
            expireAll(hot_head);
            expireAll(cold_head);
            free(memory_pool);
        }
        ~SLRUCache() {
            // user must call destruct() manually.
        }
    };
}
#endif //TICKETSYSTEM_SLRU_H
