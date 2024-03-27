#include "common.hpp"
#include <iostream>
ScopedTimer::ScopedTimer(std::string_view name): name(name), start_time(clock_t::now()){}

ScopedTimer::~ScopedTimer() {
    auto end_time = clock_t::now();
    duration_t elapsed_time = end_time - start_time;
    if(!name.empty()){
        std::cout << std::quoted(name) << " elapsed time: ";
    }
    else{
        std::cout << "Elapsed time: ";
    }
    std::cout << elapsed_time << std::endl;
}

std::vector<pthread_t> spawn_threads(size_t size, void*(*fn)(void*)){
    std::vector<pthread_t> result;
    result.resize(size);
    for(pthread_t& tid: result){
        check_result(pthread_create(&tid, nullptr, fn, nullptr));
    }
    return result;
}


void join_threads(const std::vector<pthread_t>& tids){
    for(auto tid: tids)
        check_result(pthread_join(tid, nullptr));
}
