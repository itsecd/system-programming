#include <span>
#include <random>
#include <iostream>
#include "common.hpp"


void* A(void*){
    COUT << "A begin"<<std::endl;
    timespec t{.tv_nsec = 3000000};
    nanosleep(&t,nullptr);
    COUT << "A end"<<std::endl;
    return nullptr;
}


void* B(void*){
    COUT << "B begin"<<std::endl;
    timespec t{.tv_nsec = 6000000};
    nanosleep(&t,nullptr);
    COUT << "B end"<<std::endl;
    return nullptr;
}

void* C(void*){
    COUT << "C begin"<<std::endl;
    timespec t{.tv_nsec = 1000000};
    nanosleep(&t,nullptr);
    COUT << "C end"<<std::endl;
    return nullptr;
}


void* D(void*){
    COUT << "D begin"<<std::endl;
    timespec t{.tv_nsec = 4000000};
    nanosleep(&t,nullptr);
    COUT << "D end"<<std::endl;
    return nullptr;
}

void* E(void*){
    COUT << "E begin"<<std::endl;
    timespec t{.tv_nsec = 2000000};
    nanosleep(&t,nullptr);
    COUT << "E end"<<std::endl;
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
    check_result(pthread_barrier_init(&barrier, nullptr, 2));
    pthread_t t;
    check_result(pthread_create(&t, nullptr,  ACE, nullptr));
    BDE(nullptr);
    check_result(pthread_join(t, nullptr));
}


int main(){
    {
        ScopedTimer _{"Sequential"};
        sequential();
    }
    COUT << std::endl;
    {
        ScopedTimer _{"Parallel"};
        parallel_join();
    }
    COUT << std::endl;
    {
        ScopedTimer _{"Parallel with barrier"};
        parallel_barrier();
    }

}