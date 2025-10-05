
#ifndef EMP_RELATIVE_VECTOR
#define EMP_RELATIVE_VECTOR
#include <cstring>
#include <initializer_list>
#include <stdexcept>
#include "memory/relative_pointer.hpp"
namespace emp {
template <typename T> class RelativeVector {
private:
    RelativePointer<T> m_data;
    size_t m_capacity;
    size_t m_size;

    void increase_capacity(size_t new_capacity)
    {
        if(new_capacity == m_capacity) {
            return;
        }

        T *new_data = new T[new_capacity];
        for(size_t i = 0; i < m_size; ++i) {
            new_data[i] = m_data[i];
        }
        delete[] m_data;
        m_data = new_data;
        m_capacity = new_capacity;
    }

public:
    RelativeVector()
        : m_data(nullptr)
        , m_capacity(0)
        , m_size(0)
    {
    }

    explicit RelativeVector(size_t initial_capacity)
        : m_data(new T[initial_capacity])
        , m_capacity(initial_capacity)
        , m_size(0)
    {
    }
    RelativeVector(std::initializer_list<T> init_list)
        : m_data(new T[init_list.size()])
        , m_capacity(init_list.size())
        , m_size(0)
    {
        for(const auto &elem : init_list) {
            push_back(elem);
        }
    }
    RelativeVector(const std::vector<T> &vector)
        : m_data(new T[vector.size()])
        , m_capacity(vector.size())
        , m_size(vector.size())
    {
        memcpy(m_data, vector.data(), sizeof(T) * m_size);
    }

    ~RelativeVector() { delete[] m_data; }

    void resize(size_t new_size)
    {
        if(new_size <= m_size) {
            m_size = new_size;
            return;
        }
        size_t new_capacity = (m_capacity == 0 ? 1 : m_capacity);
        while(new_size > new_capacity) {
            new_capacity *= 2;
        }
        increase_capacity(new_capacity);
        m_size = new_size;
    }
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    void reserve(size_t extra_space)
    {
        if(m_size + extra_space <= m_capacity) {
            return;
        }
        //  reserve exactly what the user needs
        increase_capacity(m_size + extra_space);
    }

    bool empty() const { return m_size == 0; }

    void push_back(const T &value)
    {
        if(m_size == m_capacity) {
            increase_capacity(m_capacity == 0 ? 1 : m_capacity * 2);
        }
        m_data[m_size++] = value;
    }

    void pop_back()
    {
        if(m_size == 0) {
            throw std::out_of_range("PopBack called on empty vector");
        }
        --m_size;
    }

    T &operator[](size_t index)
    {
        if(index >= m_size) {
            throw std::out_of_range("Index out of range");
        }
        return m_data[index];
    }

    const T &operator[](size_t index) const
    {
        if(index >= m_size) {
            throw std::out_of_range("Index out of range");
        }
        return m_data[index];
    }

    void clear() { m_size = 0; }

    void insert(size_t index, const T &value)
    {
        if(index > m_size) {
            throw std::out_of_range("Index out of range");
        }
        if(m_size == m_capacity) {
            increase_capacity(m_capacity == 0 ? 1 : m_capacity * 2);
        }
        for(size_t i = m_size; i > index; --i) {
            m_data[i] = m_data[i - 1];
        }
        m_data[index] = value;
        ++m_size;
    }

    void erase(size_t index)
    {
        if(index >= m_size) {
            throw std::out_of_range("Index out of range");
        }
        for(size_t i = index; i < m_size - 1; ++i) {
            m_data[i] = m_data[i + 1];
        }
        --m_size;
    }

    class Iterator {
    private:
        T *ptr;

    public:
        operator T *() { return ptr; }
        explicit Iterator(T *ptr)
            : ptr(ptr)
        {
        }

        T &operator*() const { return *ptr; }
        T *operator->() const { return ptr; }

        Iterator &operator++()
        {
            ++ptr;
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        Iterator &operator--()
        {
            --ptr;
            return *this;
        }

        Iterator operator--(int)
        {
            Iterator temp = *this;
            --(*this);
            return temp;
        }

        bool operator==(const Iterator &other) const { return ptr == other.ptr; }
        bool operator!=(const Iterator &other) const { return ptr != other.ptr; }
    };

    void insert(Iterator pos, const T &value)
    {
        size_t index = pos - m_data;
        insert(index, value);
    }

    void erase(Iterator pos)
    {
        size_t index = pos - m_data;
        erase(index);
    }

    Iterator begin() { return Iterator(m_data); }
    Iterator end() { return Iterator(m_data + m_size); }

    class ConstIterator {
    private:
        const T *ptr;

    public:
        operator const T *() { return ptr; }
        explicit ConstIterator(const T *ptr)
            : ptr(ptr)
        {
        }

        const T &operator*() const { return *ptr; }
        const T *operator->() const { return ptr; }

        ConstIterator &operator++()
        {
            ++ptr;
            return *this;
        }

        ConstIterator operator++(int)
        {
            ConstIterator temp = *this;
            ++(*this);
            return temp;
        }

        ConstIterator &operator--()
        {
            --ptr;
            return *this;
        }

        ConstIterator operator--(int)
        {
            ConstIterator temp = *this;
            --(*this);
            return temp;
        }

        bool operator==(const ConstIterator &other) const { return ptr == other.ptr; }
        bool operator!=(const ConstIterator &other) const { return ptr != other.ptr; }
    };

    ConstIterator cbegin() const { return ConstIterator(m_data); }
    ConstIterator cend() const { return ConstIterator(m_data + m_size); }
};
}

#endif
