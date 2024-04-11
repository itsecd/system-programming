#include "common.hpp"
#include <pthread.h>
#include <signal.h>
#include <sys/resource.h>
#include <vector>

const long long MAX_VALUE = 1000000000000;

struct Arg{
    int nice_value;
    volatile double value;
};

void* thread_main(void* arg_){


    auto arg = (Arg*) arg_;
    auto current = nice(0);
    auto delta = arg->nice_value - current ;
    check(nice(delta));
    check_result(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, nullptr));
    for(arg->value = 0; arg->value < MAX_VALUE; ++(arg->value));

    return nullptr;
}

const size_t THREAD_COUNT = 16;

int main(){

    rlimit nice_limit{.rlim_cur = 0, .rlim_max = 0};
    check(setrlimit(RLIMIT_NICE, &nice_limit));

    cpu_set_t  set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &set ); // use 1-st core only

    std::vector<Arg> args;
    std::vector<pthread_t> threads;
    args.resize(THREAD_COUNT);
    threads.resize(THREAD_COUNT);

    for(size_t i = 0; i < THREAD_COUNT; ++i)
    {
        args[i].nice_value = i;
        args[i].value = 0;
        check_result(pthread_create(&threads[i], nullptr, thread_main, &args[i]));
    }
    timespec sleep_time{.tv_sec = 5, .tv_nsec = 0};
    nanosleep(&sleep_time, nullptr);

    for(const auto& tid: threads)
        check_result(pthread_cancel(tid));

    for(const auto& tid: threads)
        check_result(pthread_join(tid, nullptr));

    for(size_t i = 0; i < THREAD_COUNT; ++i){
        std::cout << "Thread "<< std::setw(3)<< i <<" result: " << args[i].value << std::endl;
    }

}