#ifndef _SHARED_MUTEX_H_
#define _SHARED_MUTEX_H_

#include <shared_mutex>

namespace ftl {

class [[clang::capability("shared_mutex")]] SharedMutex final {
public:
    [[clang::acquire_capability()]] void lock() {
        mutex_.lock();
    }
    [[clang::release_capability()]] void unlock() {
        mutex_.unlock();
    }

    [[clang::acquire_shared_capability()]] void lock_shared() {
        mutex_.lock_shared();
    }
    [[clang::release_shared_capability()]] void unlock_shared() {
        mutex_.unlock_shared();
    }

private:
    std::shared_mutex mutex_;
};
} // namespace ftl


#endif // _SHARED_MUTEX_H_