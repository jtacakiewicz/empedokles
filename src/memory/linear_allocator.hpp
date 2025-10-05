#ifndef EMP_LINEAR_ALLOCATOR
#define EMP_LINEAR_ALLOCATOR
#include "allocator.hpp"
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <memory>

namespace emp {

class LinearAllocator : public Allocator {
public:
    LinearAllocator(size_t sz);
    LinearAllocator(void *buffer, size_t sz);
    ~LinearAllocator();

    void *Allocate(const size_t sz, const size_t alignment) final;
    inline void Free(void *) final;
    inline void Init() final { }
    void Release();
    void Reset();
    void ZeroMem();
    void Layout();

private:
    void *m_start;
    void *m_cur;
    void *m_end;
    size_t m_size;
    size_t m_free;
    bool m_initialized;
    bool m_preAlloc;
};

}
#endif  //  EMP_LINEAR_ALLOCATOR
