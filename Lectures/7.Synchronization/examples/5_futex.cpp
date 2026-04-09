#include <atomic>
#include <cassert>
#include <sys/syscall.h>
#include <linux/futex.h>
#include "common.hpp"
static_assert(ATOMIC_LONG_LOCK_FREE == 2);

constexpr size_t THREADS_COUNT = 2;
constexpr size_t RESOURCE_USE_COUNT = 1024;



// there is no futex() wrapper in libc, so we have to write it manually
static long int  futex(uint32_t *uaddr, int futex_op, uint32_t val,
            const struct timespec *timeout = nullptr, uint32_t *uaddr2 = nullptr, uint32_t val3 = 0)
{
    return syscall(SYS_futex, uaddr, futex_op, val,
                   timeout, uaddr2, val3);
}

alignas(64)
std::atomic_ulong locking_failures = 0;


class LockBase
{
protected:
    std::atomic_uint32_t _state {};

    static constexpr uint32_t LOCKED = 1;
    static constexpr uint32_t UNLOCKED = 0;

public:
    LockBase() = default;
    LockBase(const LockBase&) = delete;
    LockBase& operator=(const LockBase& other) = delete;

    ~LockBase()
    {
        assert(_state == UNLOCKED); // detect destruction of locked object (deleting object-in-use is an error)
    }

    bool try_lock()
    {
        auto previous_state = std::atomic_exchange_explicit(&_state, LOCKED, std::memory_order::acquire);
        bool result = previous_state == UNLOCKED;
        if (!result)
            locking_failures.fetch_add(1, std::memory_order::relaxed);
        return result;
    }

    void unlock()
    {
        uint32_t prev_state = std::atomic_exchange_explicit(&_state, UNLOCKED, std::memory_order::release);
        assert(prev_state == LOCKED); // detect unlocking of a not-locked object
    }
};


class SpinLock :public LockBase
{
public:

    void lock()
    {
        while (not try_lock()){}
    }
};


class Mutex: public LockBase
{
public:
    void lock()
    {
        while (not try_lock())
        {
                auto result = futex(reinterpret_cast<uint32_t*>(&_state), FUTEX_WAIT_PRIVATE, LOCKED);
                if (result == -1 && errno != EAGAIN)
                    check(-1);
        }
    }

    void unlock()
    {
        LockBase::unlock();
        check(futex(reinterpret_cast<uint32_t*>(&_state), FUTEX_WAKE_PRIVATE, 1));
    }
};


class Resource  {
    const microseconds preparing_delay;
    const microseconds processing_delay;

    size_t _uses_left = 0;
public:
    Resource(size_t uses_count, microseconds preparing_delay, microseconds processing_delay):
        _uses_left(uses_count), preparing_delay(preparing_delay), processing_delay(processing_delay){}

    Resource(const Resource&) = delete;

    virtual void acquire_lock() = 0;
    virtual void release_lock() = 0;

    [[nodiscard]]
    bool can_use() const { return _uses_left != 0; }

    void prepare_to_process() const {
        delay(preparing_delay);
    }

    void process() {
        if (_uses_left == 0)
            throw std::logic_error("Resource is depleted");
        delay(processing_delay);
        --_uses_left;
    }

    virtual ~Resource() = default;
};


template<typename TLock>
class ResourceWithLock: public Resource
{
    TLock _lock;

public:

    using Resource::Resource;

    void acquire_lock() override
    {
        _lock.lock();
    }

    void release_lock() override
    {
        _lock.unlock();
    }
};


using ResourceWithMutex = ResourceWithLock<Mutex>;
using ResourceWithSpinLock = ResourceWithLock<SpinLock>;

void* worker_thread(void* arg) {
    Resource* resource = static_cast<Resource *>(arg);

    bool ready = false;
    bool stop = false;
    while (!stop) {
        if (!ready) {
            resource->prepare_to_process();  // make preparations in non-critical section
            ready = true;
        }

        resource->acquire_lock();
            stop = !resource->can_use();
            if (!stop) {
                resource->process();
                ready = false;
            }
        resource->release_lock();
    }
    return nullptr;
}




template<typename TResource>
void run_experiment() {
    constexpr size_t THREADS_NUMS[] {1, 2, 4, 8};
    constexpr microseconds PROCESSING_DELAYS[] {0, 100, 1000};
    constexpr unsigned PREPARING_TO_PROCESSING_COEFFS[] {0, 1, 5};

    for (auto processing_delay: PROCESSING_DELAYS) {
        for (auto preparing_delay_coeff: PREPARING_TO_PROCESSING_COEFFS) {
            for (auto num_threads: THREADS_NUMS) {
                microseconds preparing_delay = preparing_delay_coeff * processing_delay;
                if (processing_delay ==0 && preparing_delay_coeff != 0)
                    preparing_delay = preparing_delay_coeff * 100;
                TResource resource{RESOURCE_USE_COUNT, preparing_delay, processing_delay};
                ScopedTimer _{std::format("{}us / {}us / {} threads", preparing_delay, processing_delay, num_threads)};
                join_threads(spawn_threads(worker_thread, num_threads, &resource));
            }
        }
    }
}

int main(int argc, char** argv) {
    std::cout << "=================MUTEX===============" << std::endl;
    run_experiment<ResourceWithMutex>();

    std::cout << "================SPINLOCK=============" << std::endl;
    run_experiment<ResourceWithSpinLock>();
}
