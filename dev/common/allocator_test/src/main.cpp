#include "memory_pool_allocator.h"
#include <gtest/gtest.h>

using namespace dev::common;

TEST(memory_pool_allocator_tests, BasicAllocation)
{
    /* Basic allocation and deallocation*/
    MemoryPool pool;
    void* ptr = pool.allocate(16);
    ASSERT_NE(ptr, nullptr);
    pool.deallocate(ptr,16);
}
