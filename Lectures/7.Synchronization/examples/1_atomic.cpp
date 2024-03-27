#include "common.hpp"

#include <atomic>
#include <iostream>

const int MAX_VALUE =1000000;
const size_t THREADS_COUNT = 16;

static_assert(MAX_VALUE > 0);

long long volatile value = 0;
std::atomic_llong  atomic_value = 0;

void* non_atomic_fn(void*){
    for(int i = 0; i < MAX_VALUE; ++i, ++value){};
    return nullptr;
}

void* atomic_fn(void*){
    for(int i = 0; i < MAX_VALUE; ++i, ++atomic_value){};
    return nullptr;
}

int main(){

    {
        ScopedTimer s{"Non-atomic"};
        join_threads(spawn_threads(THREADS_COUNT, non_atomic_fn));
        std::cout << "Non-atomic result: " << value << std::endl;
    }

    {
        ScopedTimer s{"Atomic"};
        join_threads(spawn_threads(THREADS_COUNT, atomic_fn));
        std::cout << "Atomic result: " << atomic_value << std::endl;
    }

}
