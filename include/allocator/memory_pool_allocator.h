#include "memory_pool.h"

namespace dev
{
    /**
     * @brief 
     * A minimal allocator to request chunks of various sizes
     * from the MemoryPool.
     * @tparam T 
     */
    template<typename T, std::size_t id=1>
    class MemoryPoolAllocator{
        public:
        using value_type = T;
        using propogate_container_copy_assignment = std::true_type;
        using propogate_on_container_move_assignment = std::true_type;
        using propogate_on_container_swap = std::true_type;
        using is_always_equal = std::true_type;

        /**
         * @brief 
         * Default ctor. 
         */
        MemoryPoolAllocator() noexcept
        {}

        /**
         * @brief 
         * Create a copy of the same allocator type 
         * `Alloc<T>`.
         * @param other 
         */
        MemoryPoolAllocator(MemoryPoolAllocator const & other) noexcept
        {}

        /**
         * @brief Converting ctor.Creates a copy of a different allocator
         * type `Alloc<U>`, 
         * @tparam U 
         * @param other 
         */
        template<typename U>
        MemoryPoolAllocator(MemoryPoolAllocator<U, id> const & other) noexcept
        {}
        
        /**
         * @brief Allocate memory for `n` objects of type `T`
         * 
         * @param n Number of objects to allocate
         * @return T* Pointer to the allocated memory 
         */
        T* allocate(std::size_t n){
            return static_cast<T*>(memoryPool().allocate(n * sizeof(T)));
        }

        /**
         * @brief Deallocate memory for `n` objects of type `T`
         * 
         * @param ptr Pointer to the memory to deallocate.
         * @param n Number of objects to deallocate memory for.
         */
        void deallocate(T* ptr, std::size_t n){
            memoryPool().deallocate(ptr, n * sizeof(T));
        }
        
        /**
         * @brief Rebind the allocator to another type.
         * 
         * @tparam U The new type to rebind to.
         */
        template<typename U>
        struct rebind{
            using other = MemoryPoolAllocator<U, id>;
        };
        /**
         * @brief 
         * When `MemoryPoolAllocator<T,PoolId>` is constructed,
         * a singleton global `MemoryPool` object is allocated
         * only once at initialization. Wrapping it in a function
         * ensures the  `MemoryPoolAllocator` is stateless.
         * @return auto& 
         */
        static auto& memoryPool() { static MemoryPool<id> singleton; return singleton; }
    };

    /**
     * @brief In the context of C++ allocators, two allocators
     * compare equal if they can deallocate memory allocated by 
     * each other. This is typically required by the the C++
     * standard library to ensure that containers can safely 
     * transfer the ownership of their memory between allocators.Bucket
     * 
     * Since `MemoryPoolAllocator` uses a singleton `MemoryPool`,
     * `MemoryPoolAllocators` with the same `PoolId` should 
     * compare equal.
     */
    template<class T1, class T2, std::size_t PoolId>
    bool operator==(MemoryPoolAllocator<T1,PoolId> const & lhs, MemoryPoolAllocator<T2,PoolId>const rhs){
        return true;
    }

    /**
     * @brief MemoryPoolAllocators with different PoolId
     * should compare unequal.
     * @tparam T1 
     * @tparam T2 
     * @tparam PoolId1 
     * @tparam PoolId2 
     * @param lhs 
     * @param rhs 
     * @return true 
     * @return false 
     */
    template<class T1, class T2, std::size_t PoolId1, std::size_t PoolId2>
    bool operator!=(MemoryPoolAllocator<T1,PoolId1> const & lhs, MemoryPoolAllocator<T2,PoolId2>const rhs){
        return true;
    }
}
