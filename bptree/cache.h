//
// Created by vortox on 28/3/20.
//

#ifndef BPTREE_CACHE_H
#define BPTREE_CACHE_H

#include <functional>
#include "../include/unordered_map.h"
//#include "analysis.h"
//Debug::Count __Counter;
namespace cache{

    template <typename DiskLoc_T,typename T>
    using func_expire_t =std::function<void(DiskLoc_T,const T*)>;
    template <typename DiskLoc_T,typename T>
    using func_load_t=std::function<void(DiskLoc_T, T*)>;

    template <typename DiskLoc_T,typename T>
    class LRUCache {
    private:
        /*
         * flag for double-linked list and freelist
         */
        typedef T* DataPtr;
        const static size_t LIST_END = 0;

        struct Block {
            size_t next;
            size_t prev;
            DiskLoc_T where;
            T data;
            bool dirty_page_bit;
            Block() : next(LIST_END), prev(LIST_END),dirty_page_bit(false) {}
        };

        size_t count;
        Block* pool;
        ds::unordered_map<DiskLoc_T, size_t> table;

        size_t freelist_head;

        func_load_t<DiskLoc_T,T> f_load;
        func_expire_t<DiskLoc_T,T> f_expire;
    public:
        LRUCache(size_t block_count, func_load_t<DiskLoc_T,T> load_func, func_expire_t<DiskLoc_T,T> expire_func)
                : count(block_count), freelist_head(1), f_load(load_func), f_expire(expire_func) {
            pool = new Block[count+1];
            for (int i=1; i <count; ++i)
                pool[i].next = i+1;
            pool[count].next = LIST_END;
            pool[LIST_END].next=pool[LIST_END].prev=LIST_END;
        }

        LRUCache(const LRUCache&) = delete;

        LRUCache& operator=(const LRUCache&) = delete;

        bool remove(DiskLoc_T offset) {
            auto iter = table.find(offset);
            if (iter == table.end())return false;
            auto& block = pool[iter->second];
            pool[block.prev].next = block.next;
            pool[block.next].prev = block.prev;
            block.next = freelist_head;
            freelist_head = iter->second;
            if(pool[iter->second].dirty_page_bit) {
//                __Counter.dirty();
                f_expire(block.where, &block.data);
            }
            table.erase(offset);
            return true;
        }

        DataPtr get(DiskLoc_T offset) {
            auto iter = table.find(offset);
            if (iter != table.end()) {
                // cache hit
//                __Counter.hit();
                if (iter->second == pool[LIST_END].next) {
                    return &pool[pool[LIST_END].next].data;
                }
                auto& block = pool[iter->second];
                // remove from the ring
                pool[block.prev].next = block.next;
                pool[block.next].prev = block.prev;
                // insert
                block.next = pool[LIST_END].next;
                block.prev = LIST_END;
                pool[LIST_END].next = iter->second;
                pool[block.next].prev = iter->second;
                return &block.data;
            }
            // cache miss
//            __Counter.miss();
            if (freelist_head == LIST_END)
                if(!remove(pool[pool[LIST_END].prev].where))
                    throw std::logic_error("Cache:remove failed");
            auto tmp=pool[freelist_head].next;
            /*
             * set block the head
             * pool[freelist_head] will be assigned
             */
            pool[pool[LIST_END].next].prev = freelist_head;
            pool[freelist_head].prev = LIST_END;
            pool[freelist_head].next = pool[LIST_END].next;
            pool[LIST_END].next = freelist_head;
            f_load(offset, &pool[freelist_head].data);
            pool[freelist_head].dirty_page_bit= false;
            pool[freelist_head].where=offset;
            table[offset]=freelist_head;
            // recover freelist_head
            freelist_head=tmp;
            return &pool[pool[LIST_END].next].data;
        }

        void dirty_bit_set(DiskLoc_T offset){ pool[table[offset]].dirty_page_bit= true;}
        void destruct(){
            for (size_t index = pool[LIST_END].next; index != LIST_END; index = pool[index].next) {
                if(pool[index].dirty_page_bit)
                    f_expire(pool[index].where,&pool[index].data);
            }
            delete[] pool;
            pool= nullptr;
        }
        ~LRUCache() {
            // user must call destruct() manually.
        }
    };
}
#endif //BPTREE_CACHE_H
