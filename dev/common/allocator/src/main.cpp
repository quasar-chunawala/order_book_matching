#include "memory_pool_allocator.h"
#include <vector>
#include <list>

int main(){

    using namespace dev::common;

    std::vector<double,MemoryPoolAllocator<double>> v{1.0, 2.0, 3.0, 4.0, 5.0};
    std::list<int,MemoryPoolAllocator<int>> l{42, 17, 5};
    
    return 0;
}