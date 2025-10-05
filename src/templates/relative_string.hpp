#ifndef EMP_RELATIVE_STRING
#define EMP_RELATIVE_STRING
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <ostream>
#include <istream>
#include "graphics/utils.hpp"
#include "memory/relative_pointer.hpp"

namespace emp {
template <typename Char> class BasicRelativeString {
private:
    RelativePointer<Char> m_data;
    size_t m_length;

public:
    static constexpr size_t npos = -1U;

    BasicRelativeString()
        : m_data(new Char[1] { '\0' })
        , m_length(0)
    {
    }

    //  Constructor from C-style string
    //  use size to only use subset of str
    BasicRelativeString(const Char *str, size_t size = npos)
    {
        if(str) {
            m_length = std::min(std::strlen(str), size);
            m_data = new Char[m_length + 1];
            std::memcpy(m_data, str, sizeof(Char) * m_length);
        } else {
            m_data = new Char[1] { '\0' };
            m_length = 0;
        }
    }

    BasicRelativeString(const BasicRelativeString &other)
        : m_data(new Char[other.m_length + 1])
        , m_length(other.m_length)
    {
        std::strcpy(m_data, other.m_data);
    }
    BasicRelativeString(BasicRelativeString &&other) noexcept
        : m_data(other.m_data)
        , m_length(other.m_length)
    {
        other.m_data = nullptr;
        other.m_length = 0;
    }
    ~BasicRelativeString() { delete[] m_data; }
    bool operator==(const BasicRelativeString &other) const { return std::strcmp(m_data, other.m_data) == 0; }
    bool operator==(const char *other) const { return std::strcmp(m_data, other) == 0; }
    bool operator==(const std::string &str) const { return std::strcmp(m_data, str.c_str()) == 0; }
    BasicRelativeString &operator=(const BasicRelativeString &other)
    {
        if(this != &other) {
            delete[] m_data;
            m_length = other.m_length;
            m_data = new Char[m_length + 1];
            std::strcpy(m_data, other.m_data);
        }
        return *this;
    }
    BasicRelativeString &operator=(BasicRelativeString &&other) noexcept
    {
        if(this != &other) {
            delete[] m_data;
            m_data = other.m_data;
            m_length = other.m_length;

            other.m_data = nullptr;
            other.m_length = 0;
        }
        return *this;
    }
    BasicRelativeString &operator+=(const BasicRelativeString &other)
    {
        Char *newData = new Char[size() + other.size() + 1] {};
        std::strcpy(newData, m_data);
        std::strcat(newData, other.m_data);
        m_length = size() + other.size();
        delete[] m_data;
        m_data = newData;
        return *this;
    }
    BasicRelativeString &operator+=(Char character)
    {
        Char *newData = new Char[size() + 2] {};
        std::strcpy(newData, m_data);
        newData[size()] = character;

        m_length = size() + 1;
        delete[] m_data;
        m_data = newData;
        return *this;
    }
    BasicRelativeString operator+(const BasicRelativeString &other) const
    {
        auto result = *this;
        result += other;
        return result;
    }
    Char &operator[](size_t index)
    {
        if(index >= m_length) {
            throw std::out_of_range("Index out of range");
        }
        return m_data[index];
    }

    const Char &operator[](size_t index) const
    {
        if(index >= m_length) {
            throw std::out_of_range("Index out of range");
        }
        return m_data[index];
    }

    size_t size() const { return m_length; }

    const Char *c_str() const { return m_data; }
    const Char &front() const
    {
        if(m_length == 0) {
            throw std::out_of_range("Front of empty string inaccessible");
        }
        return *m_data;
    }
    const Char &back() const
    {
        if(m_length == 0) {
            throw std::out_of_range("Back of empty string inaccessible");
        }
        return *(m_data + m_length);
    }
    size_t find(Char character) const
    {
        for(size_t i = 0; i < size(); i++) {
            if(m_data[i] == character) {
                return i;
            }
        }
        return npos;
    }
    template <typename iterable> size_t find(iterable substr) const
    {
        size_t found_at = 0;
        size_t idx = 0;
        size_t substr_size = substr.size();
        for(size_t i = 0; i < size(); i++) {
            if(m_data[i] == substr[idx]) {
                idx++;
                if(idx == 1) {
                    found_at = i;
                }
                if(idx == substr_size) {
                    return found_at;
                }
            } else {
                idx = 0;
            }
        }
        return npos;
    }
    template <> size_t find(const char *substr) const { return find(std::string(substr)); }
    BasicRelativeString substr(size_t start, size_t size = npos)
    {
        if(start >= m_length) {
            throw std::out_of_range("substr out of range");
        }
        return BasicRelativeString(m_data + start, size);
    }

    friend std::ostream &operator<<(std::ostream &os, const BasicRelativeString &str)
    {
        os << str.m_data;
        return os;
    }
    //  Stream extraction operator (friend function)
    friend std::istream &operator>>(std::istream &is, BasicRelativeString<Char> &str)
    {
        char buffer[1024];  //  Temporary buffer
        is >> buffer;
        str = BasicRelativeString<Char>(buffer);  //  Assign the input to the String object
        return is;
    }
};
typedef BasicRelativeString<char> RelativeString;
typedef BasicRelativeString<wchar_t> RelativeWstring;
}
namespace std {
template <> struct hash<emp::RelativeString> {
    size_t operator()(const emp::RelativeString &str) const
    {
        size_t hash = 0;
        auto hasher = std::hash<char>();
        for(int i = 0; i < str.size(); i++) {
            size_t cur_hash = hasher(str[i]);
            emp::hashCombine(hash, cur_hash);
        }
        return hash;
    }
};
}  //  namespace std

#endif  //  !DEBUG
