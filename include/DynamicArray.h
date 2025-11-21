#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H

#include <memory_resource>
#include <cstddef>
#include <utility>
#include <stdexcept>
#include <type_traits>

template <typename T>
class DynamicArray {
private:
    std::pmr::polymorphic_allocator<T> allocator;

    T* data = nullptr;
    size_t sz = 0;
    size_t cap = 0;

public:

    class Iterator {
    public:
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        Iterator(pointer ptr = nullptr) : p(ptr) {}

        reference operator*() const { return *p; }
        pointer operator->() const { return p; }

        Iterator& operator++() { ++p; return *this; }
        Iterator operator++(int) { Iterator t=*this; ++p; return t; }

        bool operator==(const Iterator& other) const { return p == other.p; }
        bool operator!=(const Iterator& other) const { return p != other.p; }

    private:
        pointer p;
    };


    class ConstIterator {
    public:
        using value_type = T;
        using reference = const T&;
        using pointer = const T*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        ConstIterator(pointer ptr = nullptr) : p(ptr) {}

        reference operator*() const { return *p; }
        pointer operator->() const { return p; }

        ConstIterator& operator++() { ++p; return *this; }
        ConstIterator operator++(int) { ConstIterator t=*this; ++p; return t; }

        bool operator==(const ConstIterator& other) const { return p == other.p; }
        bool operator!=(const ConstIterator& other) const { return p != other.p; }

    private:
        pointer p;
    };

    explicit DynamicArray(std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : allocator(mr) {}

    ~DynamicArray() {
        clear();
        if (data) allocator.deallocate(data, cap);
    }

    DynamicArray(const DynamicArray&) = delete;
    DynamicArray& operator=(const DynamicArray&) = delete;

    DynamicArray(DynamicArray&& other) noexcept 
        : allocator(other.allocator.resource())  
        , data(other.data)
        , sz(other.sz)
        , cap(other.cap) 
    {
        other.data = nullptr;
        other.sz = other.cap = 0;
    }


    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this != &other) {
            clear();
            if (data) allocator.deallocate(data, cap);
            data = other.data;
            sz = other.sz;
            cap = other.cap;
            
            other.data = nullptr;
            other.sz = other.cap = 0;
        }
        return *this;
    }

    void push_back(const T& v) {
        ensure_capacity();
        allocator.construct(data + sz, v);
        ++sz;
    }

    void push_back(T&& v) {
        ensure_capacity();
        allocator.construct(data + sz, std::move(v));
        ++sz;
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        ensure_capacity();
        allocator.construct(data + sz, std::forward<Args>(args)...);
        ++sz;
    }

    size_t size() const { return sz; }
    size_t capacity() const { return cap; }

    T& operator[](size_t i) {
        return data[i];
    }
    
    const T& operator[](size_t i) const { 
        return data[i]; 
    }

    T& at(size_t i) {
        if (i >= sz) throw std::out_of_range("DynamicArray index out of range");
        return data[i];
    }
    
    const T& at(size_t i) const {
        if (i >= sz) throw std::out_of_range("DynamicArray index out of range");
        return data[i];
    }

    Iterator begin() { return Iterator(data); }
    Iterator end()   { return Iterator(data + sz); }

    ConstIterator begin() const { return ConstIterator(data); }
    ConstIterator end()   const { return ConstIterator(data + sz); }

    ConstIterator cbegin() const { return ConstIterator(data); }
    ConstIterator cend()   const { return ConstIterator(data + sz); }

    bool empty() const { return sz == 0; }

    void clear() {
        for (size_t i = 0; i < sz; ++i)
            allocator.destroy(data + i);
        sz = 0;
    }


    void erase(size_t index) {
        if (index >= sz) throw std::out_of_range("erase index out of range");
        
        allocator.destroy(data + index);
        
        for (size_t i = index; i + 1 < sz; ++i) {
            data[i] = std::move(data[i+1]);
        }
        
        if (sz > 0) {
            allocator.destroy(data + sz - 1);
        }
        --sz;
    }

    void reserve(size_t new_capacity) {
        if (new_capacity <= cap) return;
        
        T* newData = allocator.allocate(new_capacity);

        for (size_t i = 0; i < sz; ++i) {
            allocator.construct(newData + i, std::move(data[i]));
            allocator.destroy(data + i);
        }

        if (data) allocator.deallocate(data, cap);

        data = newData;
        cap = new_capacity;
    }

private:
    void ensure_capacity() {
        if (sz < cap) return;
        size_t newCap = (cap == 0) ? 2 : (cap * 2);
        reserve(newCap);
    }
};

#endif