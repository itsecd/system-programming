#include "common.hpp"

constexpr size_t THREADS_COUNT = 2;
constexpr size_t RESOURCE_USE_COUNT = 16*1024;

class Resource  {
    const nanoseconds preparing_delay;
    const nanoseconds processing_delay;

    size_t _uses_left = 0;
public:
    Resource(size_t uses_count, nanoseconds preparing_delay, nanoseconds processing_delay):
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


class ResourceWithMutex : public Resource {
    pthread_mutex_t _mutex;
public:
    explicit ResourceWithMutex(size_t uses_count,nanoseconds producing_delay ={}, nanoseconds consuming_delay={}): Resource(uses_count,producing_delay, consuming_delay) {
        check_result(pthread_mutex_init(&_mutex, NULL));
    }

    ~ResourceWithMutex() {
        check_result(pthread_mutex_destroy(&_mutex));
    }

    void acquire_lock() override {
        check_result(pthread_mutex_lock(&_mutex));
    }

    void release_lock() override {
        check_result(pthread_mutex_unlock(&_mutex));
    }
};


class ResourceWithSpinLock : public Resource {

    //alignas(64)
    pthread_spinlock_t _spin;
public:
    explicit ResourceWithSpinLock(size_t uses_count, nanoseconds producing_delay ={}, nanoseconds consuming_delay={}): Resource(uses_count, producing_delay, consuming_delay) {
        check_result(pthread_spin_init(&_spin, false));
    }

    ~ResourceWithSpinLock() {
        check_result(pthread_spin_destroy(&_spin));
    }

    void acquire_lock() override {
        check_result(pthread_spin_lock(&_spin));
    }

    void release_lock() override {
        check_result(pthread_spin_unlock(&_spin));
    }
};


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
    constexpr nanoseconds PROCESSING_DELAYS[] {0, 5'000, 100'000};
    constexpr unsigned PREPARING_TO_PROCESSING_COEFFS[] {0, 1, 5};

    for (auto processing_delay: PROCESSING_DELAYS) {
        for (auto preparing_delay_coeff: PREPARING_TO_PROCESSING_COEFFS) {
            for (auto num_threads: THREADS_NUMS) {
                nanoseconds preparing_delay = preparing_delay_coeff * processing_delay;
                if (processing_delay ==0 && preparing_delay_coeff != 0)
                    preparing_delay = preparing_delay_coeff * 100;
                TResource resource{RESOURCE_USE_COUNT, preparing_delay, processing_delay};
                ScopedTimer _{std::format("{}ns / {}ns / {} threads", preparing_delay, processing_delay, num_threads)};
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



