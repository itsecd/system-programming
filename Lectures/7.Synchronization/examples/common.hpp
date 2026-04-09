//
// Created by alexey on 26.03.24.
//

#ifndef LECTURE6_COMMON_HPP
#define LECTURE6_COMMON_HPP
#include "check.hpp"
#include "pthread.h"
#include <chrono>
#include <string_view>
#include <vector>
#include <iostream>


struct SynchronizedCout
{
    explicit SynchronizedCout();
    ~SynchronizedCout();

    template<typename T>
    friend const SynchronizedCout& operator<<(const SynchronizedCout& ss, T&& value)
    {
        std::cout <<value;
        return ss;
    }

    friend const SynchronizedCout& operator<<(const SynchronizedCout& ss, std::ostream&(*value)(std::ostream&) )
    {
        value(std::cout);
        return ss;
    }
};

#define COUT_S SynchronizedCout()

struct ScopedTimer{
    using clock_t = std::chrono::high_resolution_clock;
    using duration_t = std::chrono::duration<double, std::milli>;
    const std::string name;
    const clock_t::time_point start_time;
    explicit ScopedTimer(std::string_view  name);
    ~ScopedTimer();
};

using thread_fn = void*(*)(void*);

std::vector<pthread_t> spawn_threads(thread_fn thread_function, size_t count, void* arg = nullptr);
std::vector<pthread_t> spawn_threads(thread_fn thread_function, const std::vector<void*> &args);

[[maybe_unused]]
std::vector<void*> join_threads(const std::vector<pthread_t>& thread_ids);


using microseconds = unsigned int;

inline void delay(microseconds sleep_time){
    if (sleep_time == 0) return;
    timespec sleep_time_(sleep_time/1'000'000,sleep_time % 1'000'000 * 1000 );
    nanosleep(&sleep_time_, NULL);
}

#endif //LECTURE6_COMMON_HPP
