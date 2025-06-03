#ifndef __BUCKET_DESCRIPTORS_H
#define __BUCKET_DESCRIPTORS_H

#include <tuple>
#include <type_traits>
#include <bucket.h>
/**
 * @brief 
 * A memory pool instance is merely a collection of buckets.
 * Each bucket has two properties : BlockSize and BlockCount.
 * A collection of these properties per bucket will define 
 * an instance of the pool. 
 * 
 * A collection of such parameters is expressed through a bucket_descriptors
 * instance, which is just a static collection of bucket configs. 
 * 
 * A bucket_descriptors specialization is a concrete realization 
 * of such a collection.
 */

namespace dev{
    template<std::size_t id>
    struct bucket_descriptors{
        using type = std::tuple<>;
    };

    // Bucket configurations   
    struct bucket_cfg1{
        static constexpr std::size_t BlockSize = 1;
        static constexpr std::size_t BlockCount = 10000;
    };

    struct bucket_cfg2{
        static constexpr std::size_t BlockSize = 2;
        static constexpr std::size_t BlockCount = 10000;
    };

    struct bucket_cfg4{
        static constexpr std::size_t BlockSize = 4;
        static constexpr std::size_t BlockCount = 10000;
    };

    struct bucket_cfg8{
        static constexpr std::size_t BlockSize = 8;
        static constexpr std::size_t BlockCount = 10000;
    };

    struct bucket_cfg16{
        static constexpr std::size_t BlockSize = 16;
        static constexpr std::size_t BlockCount = 10000;
    };

    struct bucket_cfg32{
        static constexpr std::size_t BlockSize = 32;
        static constexpr std::size_t BlockCount = 10000;
    };

    struct bucket_cfg64{
        static constexpr std::size_t BlockSize = 64;
        static constexpr std::size_t BlockCount = 10000;
    };

    struct bucket_cfg128{
        static constexpr std::size_t BlockSize = 128;
        static constexpr std::size_t BlockCount = 10000;
    };

    struct bucket_cfg256{
        static constexpr std::size_t BlockSize = 256;
        static constexpr std::size_t BlockCount = 10000;
    };

    struct bucket_cfg512{
        static constexpr std::size_t BlockSize = 512;
        static constexpr std::size_t BlockCount = 10000;
    };

    struct bucket_cfg1024{
        static constexpr std::size_t BlockSize = 1024;
        static constexpr std::size_t BlockCount = 10 * 1024;
    };

    template<>
    struct bucket_descriptors<1>{
        using type = std::tuple<
            bucket_cfg1,
            bucket_cfg2,
            bucket_cfg4,
            bucket_cfg8,
            bucket_cfg16, 
            bucket_cfg32, 
            bucket_cfg64,
            bucket_cfg128,
            bucket_cfg256,
            bucket_cfg512,
            bucket_cfg1024
        >;
    };
}

#endif