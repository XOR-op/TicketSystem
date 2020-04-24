
#ifndef BPTREE_LRUBPTREE_H
#define BPTREE_LRUBPTREE_H

#include <fstream>
#include <cstring>
#include "bptree.h"
#include "cache.h"
#include "../include/file_alternative.h"
using std::ios;
namespace bptree {
    /*
     *  Cache size must be at least 4 times than the DEPTH in order to ensure work correctly.
     */


    template<typename KeyType, typename ValueType, typename WeakCmp=std::less<KeyType>>
    class LRUBPTree : public BPTree<KeyType, ValueType,WeakCmp> {
    private:
        static const size_t NO_FREE = SIZE_MAX;
        typedef Node<KeyType,ValueType>* NodePtr;
        typedef const Node<KeyType,ValueType>* ConstNodePtr;


        cache::LRUCache<DiskLoc_T ,Node<KeyType,ValueType>> cache;

        static void load(ds::File& ifs, DiskLoc_T offset, NodePtr tobe_filled);

        static void flush(ds::File& ofs, ConstNodePtr node);

        NodePtr initNode(typename Node<KeyType, ValueType>::type_t t) override;

        void saveNode(NodePtr node) override;

        NodePtr loadNode(DiskLoc_T offset) override;

        void deleteNode(NodePtr node) override;

        bool createTree(const std::string& path);

//        std::fstream file;
        ds::File file;
        size_t file_size;
        DiskLoc_T freelist_head;
    public:
        LRUBPTree(const std::string& path, size_t block_size, bool create= false);

//        LRUBPTree()=default;
//
//        void open(const std::string& path,std::size_t block_size,bool create= false){
//            if(create)
//                createTree(path);
//            file.open(path.c_str());
//        }

        ~LRUBPTree();
    };



    template<typename KeyType,typename ValueType, typename WeakCmp>
    void LRUBPTree<KeyType, ValueType,WeakCmp>::flush(ds::File& ofs, ConstNodePtr node) {
        char buffer[Node<KeyType,ValueType>::BLOCK_SIZE];
        writeBuffer(node, buffer);
        ofs.seekp(node->offset);
        if (ofs.fail())throw std::runtime_error("CacheBPTree: Can't write");
        ofs.write(buffer, Node<KeyType,ValueType>::BLOCK_SIZE);
        if (ofs.fail())throw std::runtime_error("CacheBPTree: Write failure");
    }

    template<typename KeyType,typename ValueType,typename WeakCmp>
    void LRUBPTree<KeyType,ValueType,WeakCmp>::load(ds::File& ifs, bptree::DiskLoc_T offset, NodePtr tobe_filled) {
        char buffer[Node<KeyType,ValueType>::BLOCK_SIZE];
        ifs.seekg(offset);
        if (ifs.fail())throw std::runtime_error("CacheBPTree: Can't read");
        ifs.read(buffer, Node<KeyType,ValueType>::BLOCK_SIZE);
        readBuffer(tobe_filled, buffer);
        if (ifs.fail())throw std::runtime_error("CacheBPTree: Read failure");
    }

    template<typename KeyType,typename ValueType,typename WeakCmp>
    Node<KeyType,ValueType>* LRUBPTree<KeyType,ValueType,WeakCmp>::initNode(typename bptree::Node<KeyType,ValueType>::type_t t) {
        typedef Node<KeyType,ValueType> Node;
        if (freelist_head == NO_FREE) {
            // extend file
            char block[Node::BLOCK_SIZE];
            bzero(block, Node::BLOCK_SIZE);
            Node n;
            n.type = Node::FREE;
            n.offset = file_size;
            n.next = NO_FREE;
            writeBuffer(&n, block);
            file.seekp(file_size);
            file.write(block, Node::BLOCK_SIZE);
            if (file.fail())throw std::runtime_error("CacheBPTree: initNode()");
            freelist_head = file_size;
            file_size += Node::BLOCK_SIZE;
        }
        NodePtr ptr = cache.get(freelist_head);
        freelist_head = ptr->next;
        ptr->type = t;
        return ptr;
    }

    template<typename KeyType,typename ValueType,typename WeakCmp>
    void LRUBPTree<KeyType,ValueType,WeakCmp>::saveNode(NodePtr node) {
        cache.dirty_bit_set(node->offset);
    }

    template<typename KeyType,typename ValueType,typename WeakCmp>
    Node<KeyType,ValueType>* LRUBPTree<KeyType,ValueType,WeakCmp>::loadNode(bptree::DiskLoc_T offset) {
        return cache.get(offset);
    }


    template<typename KeyType,typename ValueType,typename WeakCmp>
    void LRUBPTree<KeyType,ValueType,WeakCmp>::deleteNode(NodePtr node) {
        node->type = Node<KeyType,ValueType>::FREE;
        node->next = freelist_head;
        auto tmp = node->next;
        saveNode(node);
        cache.remove(node->offset);
        freelist_head = tmp;
    }

    template <typename KeyType,typename ValueType,typename WeakCmp>
    bool LRUBPTree<KeyType,ValueType,WeakCmp>::createTree(const std::string& path) {
        std::fstream f(path, ios::in | ios::out | ios::binary);
        if (f.is_open() || f.bad()) { return false; }
        f.close();
        f = std::fstream(path, ios::out | ios::binary);
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
        char buf[sizeof(LRUBPTree<KeyType,ValueType>::file_size)+sizeof(LRUBPTree<KeyType,ValueType>::freelist_head)+sizeof(LRUBPTree<KeyType,ValueType>::root)];
        char* ptr = buf;
        size_t size = sizeof(buf);
        DiskLoc_T free = LRUBPTree<KeyType,ValueType>::NO_FREE;
        DiskLoc_T t = Node<KeyType,ValueType>::NONE;
        write_attribute(size);
        write_attribute(free);
        write_attribute(t); // root
        write_attribute(t); // sequential_head
        f.write(buf, sizeof(buf));
        f.close();
        return true;
#undef write_attribute
    }



    template<typename KeyType,typename ValueType,typename WeakCmp>
    LRUBPTree<KeyType,ValueType,WeakCmp>::LRUBPTree(const std::string& path, size_t block_size, bool create) :
            BPTree<KeyType,ValueType,WeakCmp>(),
            cache(block_size, [this](DiskLoc_T o, NodePtr r) { load(file, o, r); }, [this](DiskLoc_T o,ConstNodePtr r) { flush(file, r); }) {
        if(create)
            createTree(path);
        file.open(path.c_str());
        char buf[sizeof(file_size)+sizeof(freelist_head)+sizeof(this->root)];
        char* ptr = buf;
        file.seekg(0);
        file.read(buf, sizeof(buf));
#define read_attribute(ATTR) memcpy((void*)&ATTR,ptr,sizeof(ATTR));ptr+=sizeof(ATTR)
        read_attribute(file_size);
        read_attribute(freelist_head);
        read_attribute(this->root);
#undef read_attribute
    }

    template<typename KeyType,typename ValueType,typename WeakCmp>
    LRUBPTree<KeyType,ValueType,WeakCmp>::~LRUBPTree() {
#define write_attribute(ATTR) memcpy(ptr,(void*)&ATTR,sizeof(ATTR));ptr+=sizeof(ATTR)
        char buf[sizeof(file_size)+sizeof(freelist_head)+sizeof(this->root)];
        char* ptr = buf;
        write_attribute(file_size);
        write_attribute(freelist_head);
        write_attribute(this->root);
        file.seekg(0);
        file.write(buf, sizeof(buf));
        cache.destruct();
        file.flush();
        file.close();
#undef write_attribute
    }
}
#endif //BPTREE_LRUBPTREE_H
