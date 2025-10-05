#ifndef EMP_RELATIVE_POINTER
#define EMP_RELATIVE_POINTER
#include <cstdint>
namespace emp {
typedef uint64_t offset_t;
template <class T> struct RelativePointer {
    T *get() const;

    bool is_equal(const RelativePointer &other) const;
    bool is_null() const;
    bool is_not_null() const;

    //  Operator overloading to give a cleaner interface
    T *operator->() const;
    T &operator*() const;
    operator T *() const;

    void set(void *raw_pointer);
    void set_null();

    RelativePointer &operator=(T *);
    RelativePointer &operator=(const RelativePointer &);

    RelativePointer() { }
    RelativePointer(T *);
    RelativePointer(const RelativePointer &);

    offset_t offset;
};
template <typename T> RelativePointer<T>::RelativePointer(const RelativePointer<T> &other)
{
    set(other.get());
}
template <typename T> RelativePointer<T>::RelativePointer(T *raw)
{
    set(raw);
}
template <typename T> RelativePointer<T> &RelativePointer<T>::operator=(const RelativePointer<T> &other)
{
    set(other.get());
    return *this;
}
template <typename T> RelativePointer<T> &RelativePointer<T>::operator=(T *raw)
{
    set(raw);
    return *this;
}
template <typename T> bool RelativePointer<T>::is_equal(const RelativePointer &other) const
{
    return get() == other.get();
}
template <typename T> RelativePointer<T>::operator T *() const
{
    return get();
}
template <typename T> bool RelativePointer<T>::is_null() const
{
    return offset == 0;
}
template <typename T> bool RelativePointer<T>::is_not_null() const
{
    return !is_null();
}
template <typename T> void RelativePointer<T>::set_null()
{
    offset = 0;
}
template <typename T> void RelativePointer<T>::set(void *raw)
{
    if(raw == nullptr) {
        offset = 0;
        return;
    }
    char *address = (char *)raw;
    offset = address - (char *)&this->offset;
}
template <typename T> inline T *RelativePointer<T>::get() const
{
    //  For debugging purposes leave the address variable.
    char *address = ((char *)&this->offset) + offset;
    return is_null() ? nullptr : (T *)address;
}

template <typename T> inline T *RelativePointer<T>::operator->() const
{
    return get();
}

template <typename T> inline T &RelativePointer<T>::operator*() const
{
    return *(get());
}
}
#endif
