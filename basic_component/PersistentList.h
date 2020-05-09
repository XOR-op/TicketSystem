//
// Created by vortox on 8/5/20.
//

#ifndef TICKETSYSTEM_PERSISTENTLIST_H
#define TICKETSYSTEM_PERSISTENTLIST_H
#include "PageManager.h"
namespace t_sys{
    const DiskLoc_T NIL=0;
    template<typename T>
    class PersistentList{
    public:
        class Iterator;
    private:
        size_t file_size;
        DiskLoc_T head,tail,freelist_head;
        PageManager pm;
        void read(Iterator& iter,DiskLoc_T offset);
        void write(Iterator& iter);
        DiskLoc_T getFreeNode();
    public:
        explicit PersistentList(const std::string& path);
        ~PersistentList();
        Iterator first(){Iterator rt;read(rt,head);return rt;}
        Iterator last(){Iterator rt;read(rt,tail);return rt;}
        void insert(DiskLoc_T node,const T& val); // insert NIL means add before head
        void push_back(const T& val){insert(tail,val);}
        DiskLoc_T remove(DiskLoc_T node);
        Iterator& remove(Iterator& iter);
    };

    template<typename T>
    class PersistentList<T>::Iterator{
    private:
        DiskLoc_T next,prev,self;
        T data;
        PersistentList* father;
    public:
        T& operator*(){return data;}
        explicit operator bool()const {return self!=NIL;}
        Iterator& operator++(){
            if(next!=NIL)father->read(*this,next);
            else self=NIL;
            return *this;
        }
        Iterator& operator--(){
            if(prev!=NIL)father->read(*this,next);
            else self=NIL;
            return *this;
        }
        /*
         * need to be called after writes completed
         */
        void flush(){father->write(*this);}
    };

    template<typename T>
    PersistentList<T>::PersistentList(const std::string& path):pm(path){
        // assert there exists a list
        // todo adjust ways to handle non-exist case
        DiskLoc_T cur=0;
        auto rd=[this,&cur](void* ptr,size_t sz){pm.read((char*)ptr,cur,sz);cur+=sz;};
        rd(file_size,sizeof(file_size));
        rd(head,sizeof(head));
        rd(tail,sizeof(tail));
        rd(freelist_head,sizeof(freelist_head));
    }

    template<typename T>
    PersistentList<T>::~PersistentList(){
        DiskLoc_T cur=0;
        auto wt=[this,&cur](void* ptr, size_t sz){pm.write((char*)ptr, cur, sz);cur+=sz;};
        wt(file_size, sizeof(file_size));
        wt(head, sizeof(head));
        wt(tail, sizeof(tail));
        wt(freelist_head,sizeof(freelist_head));
    }
    template<typename T>
    void PersistentList<T>::read(PersistentList::Iterator& iter, DiskLoc_T offset) {
        iter.self=offset;
        DiskLoc_T cur=offset;
        auto rd=[this,&cur](void* ptr,size_t sz){pm.read((char*)ptr,cur,sz);cur+=sz;};
        rd(&iter.next,sizeof(DiskLoc_T));
        rd(&iter.prev,sizeof(DiskLoc_T));
        rd(&iter.data,sizeof(T));
    }
    template<typename T>
    void PersistentList<T>::write(PersistentList::Iterator& iter) {
        DiskLoc_T cur=iter.self;
        auto wt=[this,&cur](void* ptr, size_t sz){pm.write((char*)ptr, cur, sz);cur+=sz;};
        wt(&iter.next, sizeof(DiskLoc_T));
        wt(&iter.prev, sizeof(DiskLoc_T));
        wt(&iter.data, sizeof(T));
    }
    template<typename T>
    DiskLoc_T PersistentList<T>::getFreeNode() {
        if(freelist_head!=NIL){
            auto rt=freelist_head;
            pm.read((char*)&freelist_head,freelist_head,sizeof(DiskLoc_T)); // read next
            return rt;
        } else{
            auto rt=file_size;
            file_size+=sizeof(DiskLoc_T)*2+sizeof(T); // next,prev,data
            return rt;
        }
    }
    template<typename T>
    void PersistentList<T>::insert(DiskLoc_T node, const T& val) {
        auto offset=getFreeNode();
        if(node==NIL){
            if(head!=NIL) {
                pm.write((char*) &offset, head+sizeof(DiskLoc_T), sizeof(DiskLoc_T)); // overwrite node->next->prev
            }else tail=offset;
            auto kp=offset;
            auto wt=[this,&offset](void* ptr, size_t sz){pm.write((char*)ptr, offset, sz);offset+=sz;};
            wt(&head, sizeof(DiskLoc_T));
            wt(&NIL, sizeof(DiskLoc_T));
            wt(&val, sizeof(T));
            head=kp;
        } else{
            DiskLoc_T n_next=NIL;
            pm.read((char*)&n_next,node,sizeof(DiskLoc_T));
            if(n_next!=NIL) {
                pm.write((char*) &offset, n_next+sizeof(DiskLoc_T), sizeof(DiskLoc_T)); // overwrite node->next->prev
            }else tail=offset;
            pm.write((char*)&offset,node,sizeof(DiskLoc_T)); // write node->next
            auto wt=[this,&offset](void* ptr, size_t sz){pm.write((char*)ptr, offset, sz);offset+=sz;};
            wt(&n_next, sizeof(DiskLoc_T));
            wt(&node, sizeof(DiskLoc_T));
            wt(&val, sizeof(T));
        }
    }
    template<typename T>
    DiskLoc_T PersistentList<T>::remove(DiskLoc_T node) {
        // remove node from list
        DiskLoc_T n_next=NIL,n_prev=NIL;
        pm.read((char*)&n_next,node,sizeof(DiskLoc_T));
        pm.read((char*)&n_prev,node+sizeof(DiskLoc_T),sizeof(DiskLoc_T));
        if(node!=tail){
            pm.write((char*)n_prev,n_next+sizeof(DiskLoc_T),sizeof(DiskLoc_T));
        } else tail=n_prev;
        if(node!=head){
            pm.write((char*)n_next,n_prev,sizeof(DiskLoc_T));
        } else head=n_next;
        // reuse space
        pm.write((char*)&freelist_head,node,sizeof(DiskLoc_T));
        freelist_head=node;
        return n_next;
    }
    template<typename T>
    typename PersistentList<T>::Iterator& PersistentList<T>::remove(PersistentList::Iterator& iter) {
        auto nxt=remove(iter.self);
        read(iter,nxt);
        return iter;
    }
}
#endif //TICKETSYSTEM_PERSISTENTLIST_H
