#include <iostream>
#include <atomic>
#include <vector>

/* Bounded SPSC lock-free queue.
   The main features of this queue are the following:
   - SPSC : This queue is designed to work with two threads, 
     a producer thread pushing items onto the queue and a 
     consumer thread popping items off the queue. 
   - Bounded : The queue has a fixed size. We need a method
     for checking when the queue reaches its capacity and when
     it has no elements.
   - Lock-free : This queue uses atomic types that are always 
     lock-free as long as the code executes on modern processor
     with h/w support for these instructions.
*/
namespace dev{
    template<typename T>
    class spsc_queue{
        private:
        const std::size_t m_capacity;
        std::vector<T> m_buffer;
        std::atomic<int> m_head{ 0 };
        std::atomic<int> m_tail{ 0 };

        public:
        spsc_queue()
        : m_capacity{16}
        , m_buffer{}
        {
            m_buffer.reserve(16);
        }
        spsc_queue(const spsc_queue&) = delete;
        spsc_queue& operator=(const spsc_queue&) = delete;

        spsc_queue(std::size_t runtime_capacity)
        : m_capacity{runtime_capacity}
        , m_buffer{}
        {
            m_buffer.reserve(runtime_capacity);
        }

        /* try_push(T&) - pushes an element onto the ringbuffer
           requires : Only one thread is allowed to push the data to
           the spsc queue.
           post-conditions : object will be pushed to the queue
           unless the queue is full.
           returns : true if the push operation is successful.
        */ 
        bool try_push(const T& element){
            int tail = m_tail.load(std::memory_order_acquire);
            int next_tail = (tail + 1) & (m_capacity - 1);
            if(next_tail != m_head.load(std::memory_order_acquire))
            {
                m_buffer[next_tail] = element;
                m_tail.store(next_tail, std::memory_order_release);
                return true;
            }   
            return false;
        }
        
        std::optional<T> try_pop(){
            int head = m_head.load(std::memory_order_acquire);

            if(head == m_tail.load(std::memory_order_acquire))
                return std::nullopt;

            T item = m_buffer[head];
            m_head.store((head + 1 ) & (m_capacity - 1), std::memory_order_release);
            return item;
        }
    };
}