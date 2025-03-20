#include "bucket.h"
#include "memory_pool.h"
#include <iostream>
#include <vector>

using namespace dev::common;

int main(){
    
    {
        /* Basic allocation and deallocation*/
        MemoryPool pool;
        void* ptr = pool.allocate(16);
        assert(ptr!=nullptr);
        pool.deallocate(ptr,16);
    }

    {
        /* Boundary conditions */
        MemoryPool pool;
        void* ptr1 = pool.allocate(1);
        assert(ptr1 != nullptr);
        pool.deallocate(ptr1,1);
        
        void* ptr2 = pool.allocate(1024);
        assert(ptr2 != nullptr);
        pool.deallocate(ptr2,1024);
    }

    {
        /* Multiple allocations of different sizes */
        MemoryPool pool;
        void* ptr1 = pool.allocate(16);
        void* ptr2 = pool.allocate(32);
        assert(ptr1!=nullptr);
        assert(ptr2!= nullptr);
        assert(ptr1 != ptr2);

        pool.deallocate(ptr1,16);
        pool.deallocate(ptr2,32);

    }

    {
        /* Multiple allocations from the same bucket */
        MemoryPool pool;
        void* ptr1 = pool.allocate(1);
        void* ptr2 = pool.allocate(1);
        void* ptr3 = pool.allocate(1);
        assert(ptr1!=nullptr);
        assert(ptr2!= nullptr);
        assert(ptr3 != nullptr);
        assert(ptr1 != ptr2);
        assert(ptr2 != ptr3);

        pool.deallocate(ptr1,1);
        pool.deallocate(ptr2,1);
        pool.deallocate(ptr3,1);
    }

    {
        /* Exhaustion */
        MemoryPool pool;
        std::vector<void*> allocations{};
        for(std::size_t i=0; i<10'000;++i){
            void* ptr = pool.allocate(1);
            if(ptr == nullptr){
                break;
            }
            allocations.push_back(ptr);
        }

        assert(allocations.size() == 10'000);
        for(void* ptr : allocations)
            pool.deallocate(ptr,1);
    }
    
    return 0;
}