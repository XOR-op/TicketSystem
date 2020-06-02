#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP


#include <climits>
#include <cstddef>
#include <stdexcept>

namespace ds {
    template<typename T>
    class vector {
    private:

        size_t _size;
        size_t _capacity;
        T* arr;
    public:
        class const_iterator;

        class iterator {
        private:
            friend class vector;

            friend class const_iterator;

            vector* vec;
            T* ptr;
            iterator(T* p, vector* v) : ptr(p), vec(v) {}
        public:
            iterator operator+(const int& n) const { return iterator(ptr+n, vec); }
            iterator operator-(const int& n) const { return iterator(ptr-n, vec); }
            int operator-(const iterator& rhs) const {
                if (vec != rhs.vec)throw std::invalid_argument("Wrong container");
                return ptr-rhs.ptr;
            }
            iterator& operator+=(const int& n) {
                ptr += n;
                return *this;
            }
            iterator& operator-=(const int& n) { return operator+=(-n); }
            iterator operator++(int) {
                iterator rt(*this);
                ++ptr;
                return rt;
            }
            iterator& operator++() {
                ++ptr;
                return *this;
            }
            iterator operator--(int) {
                iterator rt(*this);
                --ptr;
                return rt;
            }
            iterator& operator--() {
                ++ptr;
                return *this;
            }
            T& operator*() const { return *ptr; }
            bool operator==(const iterator& rhs) const { return vec == rhs.vec && ptr == rhs.ptr; }
            bool operator==(const const_iterator& rhs) const { return vec == rhs.vec && ptr == rhs.ptr; }
            bool operator!=(const iterator& rhs) const { return !operator==(rhs); }
            bool operator!=(const const_iterator& rhs) const { return !operator==(rhs); }
        };

        class const_iterator {
        private:
            friend class vector;

            friend class iterator;

            const vector* vec;
            const T* ptr;
            const_iterator(const T* p, const vector* v) : ptr(p), vec(v) {}
        public:
            const_iterator operator+(const int& n) const { return const_iterator(ptr+n, vec); }
            const_iterator operator-(const int& n) const { return const_iterator(ptr-n, vec); }
            int operator-(const iterator& rhs) const {
                if (vec != rhs.vec)throw std::invalid_argument("Wrong container");
                return ptr-rhs.ptr;
            }
            const_iterator& operator+=(const int& n) {
                ptr += n;
                return *this;
            }
            const_iterator& operator-=(const int& n) { return operator+=(-n); }
            const_iterator operator++(int) {
                const_iterator rt(*this);
                ++ptr;
                return rt;
            }
            const_iterator& operator++() {
                ++ptr;
                return *this;
            }
            const_iterator operator--(int) {
                const_iterator rt(*this);
                --ptr;
                return rt;
            }
            const_iterator& operator--() {
                ++ptr;
                return *this;
            }
            const T& operator*() const { return *ptr; }
            bool operator==(const iterator& rhs) const { return vec == rhs.vec && ptr == rhs.ptr; }
            bool operator==(const const_iterator& rhs) const { return vec == rhs.vec && ptr == rhs.ptr; }
            bool operator!=(const iterator& rhs) const { return !operator==(rhs); }
            bool operator!=(const const_iterator& rhs) const { return !operator==(rhs); }
        };

        vector() : _capacity(32), _size(0) {
            arr = (T*) operator new(_capacity*sizeof(T));
        }
        vector(const vector& other) : _capacity(other._capacity), _size(other._size) {
            arr = (T*) operator new(_capacity*sizeof(T));
            memcpy(arr, other.arr, sizeof(T)*other._capacity);
        }
        ~vector() {
            for (int i = 0; i < _size; ++i)arr[i].~T();
            operator delete(arr);
        }
        vector& operator=(const vector& other) {
            if (&other == this)
                return *this;
            if (_capacity < other.size()) {
                operator delete(arr);
                arr = (T*) operator new(other._capacity*sizeof(T));
            }
            _capacity = other._capacity;
            _size = other._size;
            memcpy(arr, other.arr, sizeof(T)*other._size);
            return *this;
        }
        T& at(const size_t& pos) {
            if (pos < 0 || pos >= _size)throw std::overflow_error("overflow");
            return arr[pos];
        }
        const T& at(const size_t& pos) const {
            if (pos < 0 || pos >= _size)throw std::overflow_error("overflow");
            return arr[pos];
        }
        T& operator[](const size_t& pos) {
            return at(pos);
        }
        const T& operator[](const size_t& pos) const { return at(pos); }
        const T& front() const {
            if (!_size)throw std::overflow_error("overflow");
            return arr[0];
        }
        const T& back() const {
            if (!_size)throw std::overflow_error("overflow");
            return arr[_size-1];
        }
        iterator begin() { return iterator(arr, this); }
        const_iterator cbegin() const { return const_iterator(arr, this); }
        iterator end() { return iterator(arr+_size, this); }
        const_iterator cend() const { return const_iterator(arr+_size, this); }
        bool empty() const { return !_size; }
        size_t size() const { return _size; }
        size_t capacity() const { return _capacity; }
        void clear() {
            _size = 0;
        }
        iterator insert(iterator pos, const T& value) { return insert(pos-begin(), value); }
        iterator insert(const size_t& ind, const T& value) {
            if (ind > _size)throw std::overflow_error("overflow");
            if (_size == _capacity) {
                _capacity *= 2;
                T* sub = (T*) operator new(sizeof(T)*_capacity);
                for (int i = 0; i < _size; ++i)
                    new(sub+i) T(std::move(arr[i]));
                std::swap(sub, arr);
                operator delete(sub);
            }
            for (int i = _size; i > ind; --i)
                arr[i] = arr[i-1];
            new(arr+ind) T(value);
            ++_size;
            return iterator(arr+ind, this);
        }
        iterator erase(iterator pos) {
            return erase(pos-begin());
        }
        iterator erase(const size_t& ind) {
            if (ind >= _size)throw std::overflow_error("overflow");
            for (int i = ind+1; i < _size; ++i)
                arr[i-1] = arr[i];
            --_size;
            return iterator(&arr[ind], this);
        }
        void push_back(const T& value) {
            insert(_size, value);
        }
        void pop_back() {
            if (empty())throw std::underflow_error("empty");
            --_size;
        }
    };


}

#endif
