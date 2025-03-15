#ifndef __BUCKET_H__
#define __BUCKET_H__

#include <cassert>
#include <bitset>

namespace dev::common{
    /**
     * @brief 
     * A bucket is a collection of homgenous fixed-size blocks.
     * An instance of `Bucket` has `BlockCount` blocks each of size
     * `BlockSize`. The `BucketSize = BlockSize * BlockCount`. 
     * 
     * Ref Implementation:
     * https://www.youtube.com/watch?v=l14Zkx5OXr4
     */
    class Bucket{
        public:
            const std::size_t BlockSize;
            const std::size_t BlockCount;

            Bucket(std::size_t block_size, std::size_t block_count)
            : BlockSize(block_size)
            , BlockCount(block_count)
            {
                const auto data_size = BlockSize * BlockCount;
                m_data = static_cast<std::byte*>(std::malloc(data_size));
                assert(m_data != nullptr && "Memory allocation failed");
                
                const auto ledger_size = 1 + ((BlockCount - 1) / 8);
                m_ledger = static_cast<std::bitset<8>*>(std::malloc(ledger_size));
                assert(m_ledger != nullptr && "Memory allocation for ledger failed");
        
                // Initialize the blocks and the ledger, we zero everything out
                std::memset(m_data, 0, data_size);
                std::memset(m_ledger, 0, ledger_size);
            }
    
            /**
             * @brief Destroy the Bucket:: Bucket object
             * Just free the memory allocated for the data and the ledger.
             */
            ~Bucket(){
                std::free(m_data);
                std::free(m_ledger);
            }
    
            /**
             * @brief The `allocate` takes the amount of bytes,
             * calculates the amount of blocks it would take. 
             * 
             * If we don't have enough contigous blocks, we return
             * `nullptr`, otherwise we set the ledger bit to 1.
             */
            void* allocate(std::size_t bytes){
                std::size_t num_blocks = 1 + ( (bytes - 1)/ BlockSize );
        
                std::size_t next_free_index = find_contiguous_blocks(num_blocks);
        
                set_blocks_in_use(next_free_index, num_blocks);
        
                return m_data + (next_free_index * BlockSize);
            }
    
            /**
             * @brief 
             * The `deallocate` function takes a pointer(starting memory-address) 
             * to the first block in this bucket 
             * and the number of bytes we wish to free, 
             * sets the ledger bits of all corresponding blocks to 0.
             * @param ptr 
             * @param bytes 
             */
            void deallocate(void* ptr, std::size_t bytes) noexcept{
                std::size_t num_blocks = 1 + ((bytes - 1)/BlockSize);
                
                std::byte* block_offset = static_cast<std::byte*>(ptr);
                std::size_t distance = (block_offset - m_data);
                std::size_t index = distance/BlockSize;
    
                // Update the ledger
                set_blocks_free(index, num_blocks);
            }
    
            std::size_t find_contiguous_blocks(std::size_t n) noexcept{
                
                int num_free_blocks_found_ctr{0};
                std::size_t index{0};
                std::size_t first_free_block_index_in_free_area_found{0};
        
                for(int i{0};i < BlockCount; ++i){
                    if(m_ledger[i].all())
                            continue;
        
                    for(int j{0}; j<BlockSize; ++j){
                        if(!m_ledger[i].test(j))
                            ++num_free_blocks_found_ctr;
                        else{
                            num_free_blocks_found_ctr = 0;
                            first_free_block_index_in_free_area_found=index;
                        }
                            
                        
                        if(num_free_blocks_found_ctr >= n)
                            return first_free_block_index_in_free_area_found;
        
                        index++;
                    }
                }
        
                return BlockCount;
            }
    
            void set_blocks_status(
                std::size_t index, 
                std::size_t n, 
                bool set_or_clear_flag) noexcept
            {
                
                std::size_t ledger_index = (index / 8);
        
                // We are interested to flip `n` bits beginning from
                // (m_ledger + ledger_index * 8 + first_bit_to_set) to
                // (m_ledger + ledger_index * 8 + first_bit_to_set + (n - 1)).
                std::size_t first_bit_to_set = index - (ledger_index * 8);
        
                std::size_t i{0};
        
                // bitset_index follows modulo-8 arithmetic
                std::size_t bitset_index{first_bit_to_set};
        
                while(i < n)
                    m_ledger[ledger_index].set(bitset_index, set_or_clear_flag);
                    // Bitwise & is much faster than modulo, which can take
                    // 40-50 CPU cycles. a % b where b = 2^n, can be implemented
                    // as (a & (b - 1)).
                    bitset_index = (bitset_index + 1) & 7u;
                    if(bitset_index == 0)
                        ++ledger_index;
                    
                    ++i;
            }
        
            void set_blocks_free(std::size_t index, std::size_t n) noexcept{
                set_blocks_status(index, n, false);
            }
        
            void set_blocks_in_use(std::size_t index, std::size_t n) noexcept{
                set_blocks_status(index, n, true);
            }

        private:
            /**
             * @brief 
             * Finds `n` free contigous blocks in the bucket and returns the first
             * block's index or `BlockCount` on failure.
             * @param n 
             * Number of free contigous blocks requested.
             * @return std::size_t 
             */
            std::size_t find_contiguous_blocks(std::size_t n) noexcept;

            /**
             * @brief Marks `n` blocks in the ledger as in-use starting 
             * at the `index`.
             * @param index 
             * @param n 
             */
            void set_blocks_in_use(std::size_t index, std::size_t n) noexcept;

            /**
             * @brief Marks `n` blocks in the ledger as free starting
             * at the `index`.
             * @param index 
             * @param n 
             */
            void set_blocks_free(std::size_t index, std::size_t n) noexcept;
            
            void set_blocks_status(std::size_t index, std::size_t n, 
                bool set_or_clear_flag) noexcept;
            /**
             * @brief 
             * The pointer to data, which is the memory area itself which
             * we are going to allocate for our blocks.
             */
            std::byte* m_data{nullptr};

            /**
             * @brief A ledger is just a book-keeping mechanism which uses
             * one bit per block to indicate whether it is in use. So, for
             * example, if we allocate block-5 inside `m_data` array, we are
             * going to set bit 5 inside `m_ledger` to `1`. If we deallocate it,
             * we are going to clear it to `0`.
             */
            std::bitset<8>* m_ledger{nullptr};

    };
}


#endif