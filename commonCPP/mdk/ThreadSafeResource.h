#pragma once

//Most STL/boost containers are guaranteed to be thread safe over multiple reads (defined as access to const methods) but a thread needs a unique lock to call a non-const method...
template <typename TResource, typename MutexType, typename SharedLockType, typename UniqueLockType>
class ThreadSafeResourceImpl
{
    TResource mResource;
    mutable MutexType mMutex;

    //using auto keyword to access this is legit c++ (it's a private nested class, but gets returned from public methods)
    template<typename LockType, typename TResourceConstnessType>
    class TResourceLock
    {
        const LockType mLock;
    public:
        TResourceLock(TResourceConstnessType& Resource, MutexType& Mutex) : mResource(Resource), mLock(Mutex) {}
        TResourceLock(TResourceLock&& other) : mLock(std::move(other.mLock)), mResource(other.mResource) {};
        TResourceLock(const TResourceLock& other) = delete;

        TResourceConstnessType& mResource;
    };

public:
    //Gets a shared_lock over a class that only exposes const reference to the resource
    TResourceLock<SharedLockType, const TResource> GetResourceForRead() const
    {
        return TResourceLock<SharedLockType, const TResource>(mResource, mMutex);
    }

    //Gets a unique_lock over a class that exposes non-const reference to the resource
    TResourceLock<UniqueLockType, TResource> GetResourceForWrite()
    {
        return TResourceLock<UniqueLockType, TResource>(mResource, mMutex);
    }
};

#include <shared_mutex>
// note: on MSVC this uses SRW locks internally (latest WINAPI)
template<typename TResource>
using ThreadSafeResource = ThreadSafeResourceImpl<TResource, std::shared_mutex, std::shared_lock<std::shared_mutex>, std::unique_lock<std::shared_mutex>>;

