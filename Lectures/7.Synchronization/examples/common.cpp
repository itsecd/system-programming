#include "common.hpp"

#include <assert.h>
#include <iostream>
#include <iomanip>
#include <atomic>


static pthread_mutex_t cout_mutex = PTHREAD_MUTEX_INITIALIZER;

SynchronizedCout::SynchronizedCout()
{
    check_result(pthread_mutex_lock(&cout_mutex));
}

SynchronizedCout::~SynchronizedCout()
{
    pthread_mutex_unlock(&cout_mutex);
}


ScopedTimer::ScopedTimer(std::string_view name): name(name), start_time(clock_t::now())
{
    std::atomic_thread_fence(std::memory_order::acquire);
}

ScopedTimer::~ScopedTimer() {
    std::atomic_thread_fence(std::memory_order::release);
    auto end_time = clock_t::now();
    duration_t elapsed_time = end_time - start_time;
    if(!name.empty()){
        COUT_S << std::quoted(name) << " elapsed time: ";
    }
    else{
        COUT_S << "Elapsed time: ";
    }
    COUT_S << elapsed_time << std::endl;
}

std::vector<pthread_t> spawn_threads(thread_fn thread_function, size_t size, void *arg){
    std::vector<pthread_t> result;
    result.resize(size);
    for(pthread_t& tid: result){
        check_result(pthread_create(&tid, nullptr, thread_function, arg));
    }
    return result;
}

std::vector<pthread_t> spawn_threads(thread_fn thread_function, const std::vector<void*> &args)
{
    std::vector<pthread_t> result;
    result.reserve(args.size());
    for (auto arg: args)
    {
        pthread_t tid;
        check_result(pthread_create(&tid, nullptr, thread_function, arg));
        result.push_back(tid);
    }
    return result;
}


std::vector<void*> join_threads(const std::vector<pthread_t>& tids){
    std::vector<void*> results;

    for(auto& tid: tids)
    {
        void* thread_result;
        check_result(pthread_join(tid, &thread_result));
        results.push_back(thread_result);
    }

    return results;
}

void delay(const timespec& delay_time)
{
    if (delay_time.tv_nsec || delay_time.tv_sec)
        nanosleep(&delay_time, nullptr);
}