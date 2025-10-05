//  from: https://github.com/mtrebi/memory-allocators/blob/master/src/PoolAllocator.cpp
#include "pool_allocator.hpp"
#include "debug/debug.hpp"
#include "debug/log.hpp"
#include <algorithm>
#include <cassert>

namespace emp {
PoolAllocator::PoolAllocator(const std::size_t totalSize, const std::size_t chunkSize)
    : Allocator(totalSize)
{
    assert(chunkSize >= sizeof(Node) && "Object cannot be smaller than Node");
    assert(chunkSize >= 8 && "Chunk size must be greater or equal to 8");
    assert(totalSize % chunkSize == 0 && "Total Size must be a multiple of Chunk Size");
    this->m_chunkSize = chunkSize;
}

void PoolAllocator::Init()
{
    m_start_ptr = malloc(m_totalSize);
    EMP_LOG(LogLevel::DEBUG3) << "A[INIT]" << "\t@S " << m_start_ptr << "\tM" << m_totalSize;
    this->Reset();
}

void PoolAllocator::Cleanup()
{
    if(m_start_ptr != nullptr) {
        free(m_start_ptr);
    }
    m_start_ptr = nullptr;
}
PoolAllocator::~PoolAllocator()
{
    Cleanup();
}

void *PoolAllocator::Allocate(const std::size_t allocationSize, const std::size_t alignment)
{
    assert(allocationSize == this->m_chunkSize && "Allocation size must be equal to chunk size");

    Node *freePosition = m_freeList.pop();

    assert(freePosition != nullptr && "The pool allocator is full");

    m_used += m_chunkSize;
    m_peak = std::max(m_peak, m_used);
    EMP_LOG(LogLevel::DEBUG3) << "A" << "\t@S " << m_start_ptr << "\t@R " << (void *)freePosition << "\tM " << m_used;

    return (void *)freePosition;
}

void PoolAllocator::Free(void *ptr)
{
    m_used -= m_chunkSize;

    m_freeList.push((Node *)ptr);

    EMP_LOG(LogLevel::DEBUG3) << "F" << "\t@S " << m_start_ptr << "\t@F " << ptr << "\tM " << m_used;
}

void PoolAllocator::Reset()
{
    m_used = 0;
    m_peak = 0;
    //  Create a linked-list with all free positions
    const int nChunks = m_totalSize / m_chunkSize;
    for(int i = 0; i < nChunks; ++i) {
        std::size_t address = (std::size_t)m_start_ptr + i * m_chunkSize;
        m_freeList.push((Node *)address);
    }
}
}
