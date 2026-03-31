#include <span>
#include <random>
#include "common.hpp"


constexpr nanoseconds A_DELAY = 5'000'000,
                      B_DELAY = 5'000'000,
                      C_DELAY = 5'000'000,
                      D_DELAY = 5'000'000,
                      E_DELAY = 5'000'000;


#define MAKE_FN(NAME)  \
void* NAME (void*) {\
COUT_S << #NAME " start"<<std::endl; \
delay(NAME ## _DELAY);\
COUT_S << #NAME " end"<<std::endl; \
return nullptr;} \

MAKE_FN(A);
MAKE_FN(B);
MAKE_FN(C);
MAKE_FN(D);
MAKE_FN(E);

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
    std::cout << std::endl;
    {
        ScopedTimer _{"Parallel"};
        parallel_join();
    }
    std::cout << std::endl;
    {
        ScopedTimer _{"Parallel with barrier"};
        parallel_barrier();
    }

}