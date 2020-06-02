//
// Created by vortox on 11/3/20.
//

#ifndef UNORDERED_MAP_UNORDERED_MAP_H
#define UNORDERED_MAP_UNORDERED_MAP_H

#include <functional>

using std::size_t;
namespace ds {
    template<typename K, typename V>
    class SerialMap;

    template<typename value_t, typename ptr_t, typename ref_t>
    class __iterator {
    private:
        ptr_t iter;
    public:
        __iterator(ptr_t ptr) : iter(ptr) {}
        __iterator(const __iterator& other) = default;
        ref_t operator*() { return *iter; }
        ptr_t operator->() { return iter; }
        bool operator==(const __iterator& rhs) { return iter == rhs.iter; }
        bool operator!=(const __iterator& rhs) { return !(*this == rhs); }
    };

    template<typename K, typename V>
    struct __node {
        std::pair<K, V> val;
        __node* next;
        __node(const K& k, const V& v) : val(k, v), next(nullptr) {}
    };

    template<typename K, typename V, typename Allocator=std::allocator<__node<K, V>>>
    class __linked_list {
    private:
        friend class ds::SerialMap<K,V>;
        typedef std::pair<K, V> pair_t;
        typedef __node<K, V> node;
        Allocator allocator;
        node* head;
    public:
        __linked_list() : head(nullptr) {}
        ~__linked_list() {
            node* cur = head, * t;
            while (cur) {
                t = cur;
                cur = cur->next;
                delete t;
            }
        }
        pair_t* get(const K& k) {
            node* ptr = head;
            while (ptr && !(ptr->val.first == k))ptr = ptr->next;
            return ptr ? &ptr->val : nullptr;
        }
        void insert(const K& k, const V& v) {
            node* cur = new node(k, v);
            cur->next = head;
            head = cur;
        }
        bool remove(const K& k) {
            if (!head)
                return false;
            if (head->val.first == k) {
                node* tmp = head->next;
                delete head;
                head = tmp;
                return true;
            }
            node* ptr = head;
            while (ptr->next && !(ptr->next->val.first == k))ptr = ptr->next;
            if (ptr->next) {
                node* tmp = ptr->next->next;
                delete ptr->next;
                ptr->next = tmp;
                return true;
            } else
                return false;

        }
    };

    template<typename Key, typename T, class Hash=std::hash<Key>>
    class unordered_map {
    private:
        friend class ds::SerialMap<Key,T>;
        typedef std::pair<Key, T> inner_t;
        typedef __iterator<inner_t, inner_t*, inner_t&> iterator;
        Hash hash;
        size_t __bucket_count;
        __linked_list<Key, T>* buckets;
        size_t sz;
    public:
        explicit unordered_map(size_t bucket_count) : __bucket_count(bucket_count),sz(0) {
            buckets = new __linked_list<Key, T>[bucket_count];
        }
        unordered_map() : unordered_map(32) {}
        unordered_map(const unordered_map& other) = delete;
        unordered_map(unordered_map&& other) = delete;

        ~unordered_map() {
            delete[] buckets;
        }
        unordered_map& operator=(const unordered_map& other) = delete;

        iterator end() const { return iterator(nullptr); }

        T& operator[](const Key& key) {
            auto& ref=buckets[hash(key)%__bucket_count];
            if (auto ptr = ref.get(key)){
                return ptr->second;
            } else{
                ++sz;
                ref.insert(key,T());
                return ref.get(key)->second;
            }
        }
        iterator find(const Key& key) const { return iterator(buckets[hash(key)%__bucket_count].get(key)); }
        size_t erase(const Key& key) {--sz; return buckets[hash(key)%__bucket_count].remove(key); }
        size_t size()const {return sz;}
    };
}
#endif //UNORDERED_MAP_UNORDERED_MAP_H
