#include <iostream>
#include <atomic>
#include <type_traits>
#include <concepts>
#include <vector>
#include <optional>

namespace dev{
    template<typename T>
    concept Queueable = requires(T x){
        std::is_default_constructible_v<T>;
        std::is_move_constructible_v<T> || std::is_default_constructible_v<T>;
    };

    /**
     * @brief The `spsc_queue` class provides a single-reader, single-writer
     * fifo queue. 
     */
    template<Queueable T>
    class spsc_queue{
        private:
        using size_type = std::size_t;
        using value_type = T;
        using reference = T&;

        const std::size_t m_capacity;
        std::vector<T> m_buffer;
        std::atomic<int> m_read_index{ 0 };
        std::atomic<int> m_write_index{ 0 };

        public:
        spsc_queue() : spsc_queue(0){}
        spsc_queue(const spsc_queue&) = delete;
        spsc_queue& operator=(const spsc_queue&) = delete;

        spsc_queue(std::size_t runtime_capacity)
        : m_capacity{runtime_capacity}
        , m_buffer{std::vector<T>(runtime_capacity, T())}
        {}
        /**
         * @brief pushes an element onto the ringbuffer.
         * @param `element` will be pushed to the queue unless the queue is not full
         */
        template<typename U>
        requires std::is_same_v<T,U>
        bool try_push(U&& element){
            int write_index = m_write_index.load(std::memory_order_relaxed);
            int next_write_index = (write_index + 1) & (m_capacity - 1);
            if(next_write_index != m_read_index.load(std::memory_order_acquire))
            {
                m_buffer[write_index] = std::forward<U>(element);
                m_write_index.store(next_write_index, std::memory_order_release);
                return true;
            }   
            return false;
        }
        
        std::optional<T> try_pop(){
            int read_index = m_read_index.load(std::memory_order_relaxed);

            if(read_index == m_write_index.load(std::memory_order_acquire))
                return std::nullopt;

            T item = m_buffer[read_index];
            m_read_index.store((read_index + 1 ) & (m_capacity - 1), std::memory_order_release);
            return item;
        }
    };
}