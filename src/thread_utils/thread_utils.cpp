#include <iostream>
#include <atomic>
#include <thread>
#include <unistd.h>
#include <sys/syscall.h>

namespace dev::common{
    /**
     * @brief 
     * A helper-function to set the CPU affinity mask of the 
     * `this_thread`. 
     * 
     * The `cpu_set_t` data-structure represents a set of CPUs,
     * implemented as a bit mask. `CPU_ZERO()` macro clears the CPU set,
     * `CPU_SET()` macro adds cpu to the CPU set. 
     * 
     * A thread's CPU affinity mask determines the set of CPUs on which
     * it is eligible to run. On a multiprocessor system, setting the 
     * CPU affinity mask can be used to obtain performance benefits.
     * For example, by dedicating one CPU to a particular thread (i.e.
     * setting the affinity mask of that thread to specify a single CPU),
     * it is possible to ensure maximum execution speed for that thread.
     * Restricting a thread to run on a single CPU also avoids the 
     * performance cost caused by cache invalidation that occurs
     * when a thread ceases to execute on one CPU and then recommences
     * execution on a different CPU. 
     * 
     * A CPU affinity mask is represented by the `cpu_set_t` structure, 
     * the "CPU set" pointed to by the mask.
     * 
     * @param core_id 
     * @return auto 
     */
    inline auto setThreadCore(int core_id) noexcept{
        cpu_set_t cpuset;

        // clear the cpu set struct
        CPU_ZERO(&cpuset);  

        // set the cpuset to core_id
        CPU_SET(core_id, &cpuset);

        return (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) == 0);
    }
    /**
     * @brief 
     * This method creates a new thread instance, sets
     * the thread affinity on the thread and forwards the function
     * and the related arguments that the thread will run during 
     * its execution.
     * @tparam Func Function type.
     * @tparam Args Variadic template parameter representing type of args to func().
     * @param core_id The CPU core to which we would like to pin this thread.
     * @param name Thread name.
     * @param func The work package to execute.
     * @param args Arguments to `func()`
     * @return auto 
     */
    template <typename Func, typename... Args>
    inline auto createAndStartThread(
        int core_id, 
        const std::string& name,
        Func&& func,
        Args&&... args
    ){
        std::atomic<bool> running(false), failed(false);
        auto thread_body = [&](){
            if(core_id >= 0 && !setThreadCore(core_id)){
                std::cerr << "\n" << "Failed to set core affinity for " <<
                    name << " " << pthread_self() << " to " << core_id <<
                    std::endl;
                failed = true;
                return;
            }

            std::cout << "Set core affinity for " << name << " " << 
            pthread_self() << " to " << core_id << std::endl;
            running = true;

            // Invoke the work-package
            std::forward<Func>(func)(std::forward<Args>(args)...);
        };

        auto t = new std::thread(thread_body);
        while(!running && !failed){
            std::this_thread::sleep_for(std::literals::chrono_literals(1s));
        }

        if(failed){
            t->join();
            delete t;
            t = nullptr;
        }

        return t;
    }
};

