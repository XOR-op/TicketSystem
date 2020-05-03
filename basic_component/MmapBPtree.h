//
// Created by vortox on 27/4/20.
//

#ifndef BPTREE_MMAPBPTREE_H
#define BPTREE_MMAPBPTREE_H

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <unistd.h>
#include "bptree.h"

using std::ios;
namespace bptree {


    template<typename KeyType, typename ValueType, typename WeakCmp=std::less<KeyType>>
    class MmapBPTree : public BPTree<KeyType, ValueType, WeakCmp> {
    private:
        enum {
            SIZE_O = 0, FREE_HEAD_O = 8, ROOT_O = 16
        };
        static const size_t NO_FREE = SIZE_MAX;
        static const size_t MAX_SIZE = 1024*1024*512;
        typedef Node <KeyType, ValueType>* NodePtr;
        typedef const Node <KeyType, ValueType>* ConstNodePtr;

        NodePtr initNode(typename Node<KeyType, ValueType>::type_t t) override;

        void saveNode(NodePtr node) override;

        NodePtr loadNode(DiskLoc_T offset) override;

        void deleteNode(NodePtr node) override;

        bool createTree(const std::string& path);

        int fd;
        void* base;
        size_t file_size;
        DiskLoc_T freelist_head;

        template<typename T>
        T& at(DiskLoc_T offset) { return *(T*) ((char*) base+offset); }
    public:
        explicit MmapBPTree(const std::string& path, bool create = false, const WeakCmp& cmp = WeakCmp());
        ~MmapBPTree();
    };


    template<typename KeyType, typename ValueType, typename WeakCmp>
    Node <KeyType, ValueType>*
    MmapBPTree<KeyType, ValueType, WeakCmp>::initNode(typename bptree::Node<KeyType, ValueType>::type_t t) {
        typedef Node<KeyType, ValueType> Node;
        if (freelist_head == NO_FREE) {
            ftruncate(fd, file_size+sizeof(Node));
            NodePtr ptr = &at<Node>(file_size);
            ptr->offset = file_size, ptr->next = NO_FREE;
            freelist_head = file_size;
            file_size +=sizeof(Node);
            assert(ptr->offset<1024*1024*20);
        }
        NodePtr ptr = &at<Node>(freelist_head);
        ptr->type = t;
        freelist_head = ptr->next;
        return ptr;
    }

    template<typename KeyType, typename ValueType, typename WeakCmp>
    void MmapBPTree<KeyType, ValueType, WeakCmp>::saveNode(NodePtr node) {
        // do nothing
    }

    template<typename KeyType, typename ValueType, typename WeakCmp>
    Node<KeyType, ValueType>* MmapBPTree<KeyType, ValueType, WeakCmp>::loadNode(bptree::DiskLoc_T offset) {
        return &at<Node<KeyType,ValueType>>(offset);
    }


    template<typename KeyType, typename ValueType, typename WeakCmp>
    void MmapBPTree<KeyType, ValueType, WeakCmp>::deleteNode(NodePtr node) {
        node->type = Node<KeyType, ValueType>::FREE;
        node->next = freelist_head;
        freelist_head = node->offset;
    }

    template<typename KeyType, typename ValueType, typename WeakCmp>
    bool MmapBPTree<KeyType, ValueType, WeakCmp>::createTree(const std::string& path) {
        std::fstream f(path, ios::in | ios::out | ios::binary);
        if (f.is_open() || f.bad()) { return false; }
        f.close();
        f = std::fstream(path, ios::out | ios::binary);
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
        char buf[
                sizeof(MmapBPTree<KeyType, ValueType>::file_size)+sizeof(MmapBPTree<KeyType, ValueType>::freelist_head)+
                sizeof(MmapBPTree<KeyType, ValueType>::root)];
        char* ptr = buf;
        size_t size = sizeof(buf);
        DiskLoc_T free = MmapBPTree<KeyType, ValueType>::NO_FREE;
        DiskLoc_T t = Node<KeyType, ValueType>::NONE;
        write_attribute(size);
        write_attribute(free);
        write_attribute(t); // root
        f.write(buf, sizeof(buf));
        f.close();
        return true;
#undef write_attribute
    }

    template<typename KeyType, typename ValueType, typename WeakCmp>
    MmapBPTree<KeyType, ValueType, WeakCmp>::MmapBPTree(const std::string& path, bool create, const WeakCmp& cmp)
            :BPTree<KeyType, ValueType, WeakCmp>(cmp) {
        if (create)createTree(path);
        fd = open(path.c_str(), O_RDWR);
        base = mmap(nullptr, MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED , fd, 0);
        file_size = at<size_t>(SIZE_O);
        freelist_head = at<DiskLoc_T>(FREE_HEAD_O);
        this->root = at<DiskLoc_T>(ROOT_O);
    }

    template<typename KeyType, typename ValueType, typename WeakCmp>
    MmapBPTree<KeyType, ValueType, WeakCmp>::~MmapBPTree() {
        at<size_t>(SIZE_O) = file_size;
        at<DiskLoc_T>(FREE_HEAD_O) = freelist_head;
        at<DiskLoc_T>(ROOT_O) = this->root;
        munmap(base, MAX_SIZE);
        close(fd);
    }


}
#endif //BPTREE_MMAPBPTREE_H
