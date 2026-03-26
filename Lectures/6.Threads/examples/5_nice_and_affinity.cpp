#include "common.hpp"
#include <pthread.h>
#include <signal.h>
#include <sys/resource.h>
#include <vector>

constexpr size_t THREAD_COUNT = 16;
static_assert(THREAD_COUNT < 20);

constexpr timespec WORK_TIME {5, 0};

constexpr size_t NUM_CORES = 1; // 0 means no limit
static_assert( NUM_CORES < CPU_SETSIZE);

struct Arg{
    int nice_value;
    volatile unsigned long long value;
};

volatile bool start = false;
volatile bool stop = false;

void* thread_main(void* arg_){
    while(!start){}
    auto arg = (Arg*) arg_;
    auto current = nice(0);
    auto delta = arg->nice_value - current ;
    check(nice(delta));
    while(!stop) ++(arg->value);

    return nullptr;
}

int main(){

    rlimit nice_limit{.rlim_cur = 0, .rlim_max = 0};
    check(setrlimit(RLIMIT_NICE, &nice_limit));

    if (NUM_CORES > 0)
    {
        size_t num_cores_available = check(sysconf(_SC_NPROCESSORS_CONF));
        size_t num_cores = NUM_CORES < num_cores_available ? NUM_CORES : num_cores_available;
        cpu_set_t  set;
        CPU_ZERO(&set);
        // CPU_SET(0, &set);
        for (size_t i = 0; i < num_cores; ++i)
            CPU_SET(i, &set);
        check(pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &set )); // pin to selected cores
    }

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
    start = true;
    nanosleep(&WORK_TIME, nullptr);

    stop = true;
    for(const auto& tid: threads)
        check_result(pthread_join(tid, nullptr));

    for(size_t i = 0; i < THREAD_COUNT; ++i){
        std::cout << "Thread "<< std::setw(3)<< i <<" result: " << args[i].value << std::endl;
    }

}