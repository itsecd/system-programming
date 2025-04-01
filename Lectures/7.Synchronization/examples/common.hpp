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

#include <syncstream>

#define COUT std::osyncstream(std::cout)

struct ScopedTimer{
    using clock_t = std::chrono::high_resolution_clock;
    using duration_t = std::chrono::duration<double, std::milli>;
    const std::string name;
    const clock_t::time_point start_time;
    explicit ScopedTimer(std::string_view  name);
    ~ScopedTimer();

};

std::vector<pthread_t> spawn_threads(size_t size, void*(*fn)(void*));
void join_threads(const std::vector<pthread_t>& tids);

#endif //LECTURE6_COMMON_HPP
