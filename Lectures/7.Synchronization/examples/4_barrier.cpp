#include <span>
#include <random>
#include <iostream>
#include "common.hpp"


void* A(void*){
    timespec t{.tv_nsec = 300000};
    nanosleep(&t,nullptr);
    return nullptr;
}


void* B(void*){
    timespec t{.tv_nsec = 600000};
    nanosleep(&t,nullptr);
    return nullptr;
}

void* C(void*){
    timespec t{.tv_nsec = 100000};
    nanosleep(&t,nullptr);
    return nullptr;
}


void* D(void*){
    timespec t{.tv_nsec = 400000};
    nanosleep(&t,nullptr);
    return nullptr;
}

void* E(void*){
    timespec t{.tv_nsec = 200000};
    nanosleep(&t,nullptr);
    return nullptr;
}


pthread_barrier_t  barrier;

void* ACE(void* arg){
    A(arg);
    pthread_barrier_wait(&barrier);
    C(arg);
    if(pthread_barrier_wait(&barrier) == PTHREAD_BARRIER_SERIAL_THREAD)
        E(arg);
    return nullptr;
}

void* BDE(void* arg){
    B(arg);
    pthread_barrier_wait(&barrier);
    D(arg);
    if(pthread_barrier_wait(&barrier) == PTHREAD_BARRIER_SERIAL_THREAD)
        E(arg);
    return nullptr;
}


void sequential(){
    A(nullptr);
    B(nullptr);
    C(nullptr);
    D(nullptr);
    E(nullptr);
}

void parallel_join(){
    pthread_t t;
    check_result(pthread_create(&t, nullptr,  A, nullptr));
    B(nullptr);
    check_result(pthread_join(t, nullptr));
    check_result(pthread_create(&t, nullptr,  C, nullptr));
    D(nullptr);
    check_result(pthread_join(t, nullptr));
    E(nullptr);
}

void parallel_barrier(){
    pthread_t t;
    check_result(pthread_create(&t, nullptr,  ACE, nullptr));
    BDE(nullptr);
    check_result(pthread_join(t, nullptr));
}


int main(){
    check_result(pthread_barrier_init(&barrier, nullptr, 2));
    {
        ScopedTimer _{"Sequential"};
        sequential();
    }
    {
        ScopedTimer _{"Parallel"};
        parallel_join();
    }
    {
        ScopedTimer _{"Parallel with barrier"};
        parallel_barrier();
    }

}