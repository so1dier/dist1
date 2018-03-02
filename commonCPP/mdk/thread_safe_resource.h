#pragma once
#include <shared_mutex>
#include <atomic>
// Most STL/boost containers are guaranteed to be thread safe over
// multiple reads (defined as access to const methods) but a thread needs a unique lock to call a non-const method...

namespace mdk
{
    namespace detail
    {
        template <typename TResource, typename MutexType, typename SharedLockType, typename UniqueLockType>
        class thread_safe_resource_impl
        {
            TResource _resource;
            mutable MutexType _mutex;

            // Using auto keyword to access this is legit c++ (it's a private nested class, but gets returned from public methods)
            template<typename LockType, typename TResourceConstnessType>
            class resource_lock_t
            {
                const LockType _lock;
                TResourceConstnessType& _resource;
            public:
                resource_lock_t(TResourceConstnessType& resource, MutexType& mutex) : _lock(mutex), _resource(resource) {}
                resource_lock_t(resource_lock_t&& other) : _lock(std::move(other._lock)), resource(other._resource) {};
                resource_lock_t(const resource_lock_t& other) = delete;

                auto& r() { return _resource; }
            };

        public:
            //Gets a shared_lock over a class that only exposes const reference to the resource
            resource_lock_t<SharedLockType, const TResource> GetResourceForRead() const
            {
                return resource_lock_t<SharedLockType, const TResource>(_resource, _mutex);
            }

            //Gets a unique_lock over a class that exposes non-const reference to the resource
            resource_lock_t<UniqueLockType, TResource> GetResourceForWrite()
            {
                return resource_lock_t<UniqueLockType, TResource>(_resource, _mutex);
            }
        };
    }
    // note: on MSVC this uses SRW locks internally (latest WINAPI)
    template<typename TResource>
    using thread_safe_resource = detail::thread_safe_resource_impl<TResource, std::shared_mutex, std::shared_lock<std::shared_mutex>, std::unique_lock<std::shared_mutex>>;
}
