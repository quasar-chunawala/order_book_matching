#ifndef __MEMORY_POOL_H
#define __MEMORY_POOL_H

#include <array>
#include <bucket.h>
#include <bucket_descriptors.h>

namespace dev::common{

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
            std::tuple_element_t<BucketIdx,bucket_descriptors_t>::BlockCount
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
            std::tuple_element_t<BucketIdx,bucket_descriptors_t>::BlockSize
        >{};

        // Constructors
        template<std::size_t... Idx>
        MemoryPool(std::index_sequence<Idx...>)
            : m_buckets{(Bucket(get_size<Idx>::value,get_count<Idx>::value),...)}
            {}

        MemoryPool()
            : MemoryPool(std::make_index_sequence<10UL>){}

        private:
        std::array<Bucket,bucket_count> m_buckets;

    };

}
#endif