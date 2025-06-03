#ifndef __MEMORY_POOL_H
#define __MEMORY_POOL_H

#include <array>
#include <initializer_list>
#include <bucket.h>
#include <bucket_descriptors.h>
#include <algorithm>
#include <iostream>

/**
 * @note
 * Memory pools with fixed block sizes is a solution which address these issues:
 * - It provides constant-time (de)allocations in a preallocated buffer of memory.
 * - It avoids memory fragmentation with accurately-sized buckets
 * - It is easy to implement, understand and reason about.
 */
namespace dev{
    /**
     * @brief A helper structure that stores 
     * info on what-if we used to this bucket to allocate
     * requested bytes, and calculate the wasted memory.
     */
    struct Info{
        std::size_t index{0};   //Which bucket?
        std::size_t block_count{0}; // How many blocks would the allocation
                                    // take from this bucket?
        std::size_t waste{0};   // How much memory would be wasted?

        /**
         * @brief 
         * Comparator for two info objects.
         * @param other 
         * @return true 
         * @return false 
         */
        bool operator<(const Info& other) const noexcept{
            return (waste == other.waste) ? block_count < other.block_count : (waste < other.waste);
        }
    };
    /**
     * @brief 
     * A memory pool is an a fixed-size array of buckets.
     * A large number of bucket configurations such as
     * Buckets with BlockSize = 1 byte, 2 bytes, 4 bytes, ... and 
     * so forth are available for pre-selection as compile-time.
     * 
     * @tparam id Template parameter to select 
     * the pool configuration.
     */
    template<std::size_t id=1>
    class MemoryPool{
        public:        
        /**
         * @brief 
         * Alias template for the `bucket_descriptors<id>::type`
         * @tparam id 
         */
        using bucket_descriptors_t = typename bucket_descriptors<id>::type;

        /**
        * @brief 
        * Variable template to get the number of buckets in a 
        * memory pool-id.
        * @tparam id 
        */
        static constexpr std::size_t bucket_count = std::tuple_size_v<bucket_descriptors_t>;

        /**
         * @brief 
         * A type-metafunction to help us extract the `BlockSize` property
         * from the specified `BucketIdx`
         * @tparam Idx 
         */
        template<std::size_t BucketIdx>
        struct get_size : std::integral_constant<
            std::size_t,
            std::tuple_element_t<BucketIdx,bucket_descriptors_t>::BlockSize
            >{};

        /**
         * @brief 
         * A type-metafunction to help us extract the `BlockCount` property
         * from the specified `BucketIdx`
         * @tparam BucketIdx 
         */
        template<std::size_t BucketIdx>
        struct get_count : std::integral_constant<
            std::size_t,
            std::tuple_element_t<BucketIdx,bucket_descriptors_t>::BlockCount
        >{};

        // Constructors
        template<std::size_t... Idx>
        MemoryPool(std::index_sequence<Idx...>)
            : m_buckets{Bucket{get_size<Idx>::value, get_count<Idx>::value}...}
            {}

        MemoryPool()
            : MemoryPool(std::make_index_sequence<bucket_count>{})
            {}

        MemoryPool(MemoryPool const &) = delete; 
        MemoryPool& operator=(MemoryPool const &) = delete;

        /**
         * @note
         * When allocating from a bucket it is unknown whether the allocation
         * is for one or multiple objects - only the size in bytes is given.
         * So, we lose this information. 
         * 
         * One way to solve this problem would be to find which allocation
         * would lead to least wasted memory and will take the least amount
         * of blocks. So, we calculate the minimum amount of wasted memory
         * and the minimum amount of wasted blocks and the bucket which 
         * gives us that is going to be good enough.
         */

        void* allocate(std::size_t bytes){
            std::array<Info,bucket_count> deltas{};
            std::size_t index {0};
            for(Bucket& bucket : m_buckets){
                if(bucket.BlockSize >= bytes)
                {
                    deltas[index].waste = bucket.BlockSize - bytes;
                    deltas[index].block_count = 1;
                }
                else{
                    const auto n = 1 + (bytes - 1) / bucket.BlockSize;
                    const auto storage_required = n * bucket.BlockSize;
                    deltas[index].waste = (bytes - storage_required);
                    deltas[index].block_count = n;
                }
                deltas[index].index = index;
                ++index;
            }

            std::sort(deltas.begin(), deltas.end());

            for(const auto& d: deltas)
            {
                auto ptr = m_buckets[d.index].allocate(bytes);
                if(ptr!=nullptr)
                    return ptr;
            }

            throw std::bad_alloc{};
        }

        void deallocate(void* ptr, std::size_t bytes){
            /*auto* it = std::find_if(m_buckets.begin(), m_buckets.end(),[&](Bucket element){
                return element.belongs(ptr);
            });*/
            
            int i{0};
            for(i = 0;i<m_buckets.size();++i){
                if(m_buckets[i].belongs(ptr))
                    break;
            }

            if(i!=m_buckets.size())
                m_buckets[i].deallocate(ptr,bytes);
        }

        private:
        std::array<Bucket,bucket_count> m_buckets;

    };

}
#endif