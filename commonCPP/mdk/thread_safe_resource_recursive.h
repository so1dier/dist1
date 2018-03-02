#pragma once
#include "thread_safe_resource.h"
#include <atomic>

namespace mdk
{
    namespace detail
    {
        // shared recursive mutex implementation, so can call for_write on same thread without a dead-lock. for_write then for_read will still not work.
        struct shared_recursive_mutex : std::shared_mutex
        {
            void lock() 
            {
                const auto this_id = std::this_thread::get_id();
                if(owner == this_id) 
                {
                    // recursive locking
                    ++count;
                }
                else 
                {
                    // normal locking
                    std::shared_mutex::lock();
                    owner = this_id;
                    count = 1;
                }
            }
            
            void unlock() 
            {
                if(count > 1) 
                {
                    // recursive unlocking
                    --count;
                }
                else 
                {
                    // normal unlocking
                    owner = std::thread::id();
                    count = 0;
                    std::shared_mutex::unlock();
                }
            }

        private:
            std::atomic<std::thread::id> owner;
            size_t count = 0;
        };
    }

    template<typename TResource>
    using thread_safe_resource_recursive = detail::thread_safe_resource_impl
        <
            TResource, 
            detail::shared_recursive_mutex, 
            std::shared_lock<detail::shared_recursive_mutex>, 
            std::unique_lock<detail::shared_recursive_mutex>
        >;
}
