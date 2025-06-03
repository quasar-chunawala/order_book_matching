#include "bucket.h"
#include "memory_pool.h"
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

TEST(memory_pool_tests, BasicAllocation)
{
    /* Basic allocation and deallocation*/
    dev::MemoryPool pool;
    void* ptr = pool.allocate(16);
    ASSERT_NE(ptr, nullptr);
    pool.deallocate(ptr,16);
}

TEST(memory_pool_tests, BoundaryConditions)
{
    /* Boundary conditions */
    dev::MemoryPool pool;
    void* ptr1 = pool.allocate(1);
    ASSERT_NE(ptr1, nullptr);
    pool.deallocate(ptr1,1);
    
    void* ptr2 = pool.allocate(1024);
    ASSERT_NE(ptr2, nullptr);
    pool.deallocate(ptr2,1024);
}

TEST(memory_pool_tests, MultipleAllocations)
{
    /* Multiple allocations of different sizes */
    dev::MemoryPool pool;
    void* ptr1 = pool.allocate(16);
    void* ptr2 = pool.allocate(32);
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);

    pool.deallocate(ptr1,16);
    pool.deallocate(ptr2,32);

}

TEST(memory_pool_tests, MultipleAllocationsFromSameBucket)
{
    /* Multiple allocations from the same bucket */
    MemoryPool pool;
    void* ptr1 = pool.allocate(1);
    void* ptr2 = pool.allocate(1);
    void* ptr3 = pool.allocate(1);
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);
    ASSERT_NE(ptr1, ptr2);
    ASSERT_NE(ptr2, ptr3);

    pool.deallocate(ptr1,1);
    pool.deallocate(ptr2,1);
    pool.deallocate(ptr3,1);
}

TEST(memory_pool_tests, Exhaustion)
{
    /* Exhaustion */
    dev::MemoryPool pool;
    std::vector<void*> allocations{};
    for(std::size_t i=0; i<10'000;++i){
        void* ptr = pool.allocate(1);
        if(ptr == nullptr){
            break;
        }
        allocations.push_back(ptr);
    }

    ASSERT_EQ(allocations.size() == 10'000, true);
    for(void* ptr : allocations)
        pool.deallocate(ptr,1);
}
    
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}