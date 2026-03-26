#include "common.hpp"
#include <pthread.h>
#include <signal.h>
#include <iostream>
#include <string.h>

#include <syncstream>
#define COUT std::osyncstream(std::cout)


const int HANDLED_SIGNAL1 = SIGRTMIN;   // to thread 1 and 2
const int HANDLED_SIGNAL2 = SIGRTMIN+1; // to thread 1
const int BLOCKED_SIGNAL = SIGRTMIN+3;  // to blocked threads

constexpr int NUM_SIGNALS = 3;
constexpr timespec SIGNAL_INTERVAL{.tv_sec=0,  .tv_nsec=250'000'000};
constexpr timespec PROCESSING_TIME{.tv_sec=0,  .tv_nsec=255'000'000};

//NOTE: working with TLS is not async-signal safe! Use with caution
thread_local volatile sig_atomic_t last_signal;
thread_local volatile sig_atomic_t last_value;

void handler(int sig, siginfo_t* info, void*){
    last_signal = sig;
    last_value = info->si_value.sival_int;
}

struct Arg{
    int thread_index;
    sigset_t mask;
    int wait_signal;
};


void imitate_processing()
{
    nanosleep(&PROCESSING_TIME, nullptr);
}

void* handling_thread_main(void* arg_){
    last_signal = 0;
    auto arg = (Arg*)arg_;

    while(true) {
        check_allowed(sigsuspend(&arg->mask), EINTR);
        COUT << "Thread " << arg->thread_index << " >> " << " got signal " << std::quoted(strsignal(last_signal))
                << " with value "<< last_value << std::endl;
        imitate_processing();
    }
}

void* blocking_thread_main(void* arg_){
    last_signal = 0;
    auto arg = (Arg*)arg_;

    check(pthread_sigmask(SIG_SETMASK, &arg->mask, nullptr));

    sigset_t  wait_mask{};
    sigemptyset(&wait_mask);
    check(sigaddset(&wait_mask, arg->wait_signal));

    while(true) {
        siginfo_t si{};
        int signo = check_allowed(sigwaitinfo(&wait_mask, &si), EINTR);
        if(signo == -1) {
            COUT << "(Thread " << arg->thread_index << " EINTR)"<<std::endl;
            continue;
        }
        COUT << "Thread " << arg->thread_index << " >> " << " got blocked signal " << std::quoted(strsignal(signo))
                  << " with value "<< si.si_value.sival_int << std::endl;
        imitate_processing();
    }
}

void send_process_directed_signal(int signal, int value)
{
    COUT<<"Main thread >> Sending signal "<<std::quoted(strsignal(signal)) << " with value " << value << " to the process" << std::endl;
    check(sigqueue(getpid(), signal, {value})); // may be handled by threads 1-2
}

void send_thread_directed_signal(pthread_t thread_id, int thread_index, int signal, int value)
{
    COUT<<"Main thread >> Sending signal "<<std::quoted(strsignal(signal)) << " with value " << value << " to Thread "<< thread_index << std::endl;
    check_result(pthread_sigqueue(thread_id, signal, {value}));
}

int main(){
    sigset_t base_set{};
    sigemptyset(&base_set);
    sigaddset(&base_set, HANDLED_SIGNAL1 );
    sigaddset(&base_set, HANDLED_SIGNAL2 );
    sigaddset(&base_set, BLOCKED_SIGNAL );
    check(sigprocmask(SIG_SETMASK, &base_set, nullptr));

    pthread_t thread1, thread2, thread3, thread4;
    Arg arg1{1}, arg2{2}, arg3{3}, arg4{4};

    arg1.mask = base_set;
    sigdelset(&arg1.mask, HANDLED_SIGNAL1);
    sigdelset(&arg1.mask, HANDLED_SIGNAL2);

    arg2.mask = base_set;
    sigdelset(&arg2.mask, HANDLED_SIGNAL1);

    arg3.mask = base_set;
    arg3.wait_signal = BLOCKED_SIGNAL;

    sigfillset(&arg4.mask) ;
    arg4.wait_signal = BLOCKED_SIGNAL;

    struct sigaction s{};
    s.sa_sigaction = handler;
    s.sa_flags = SA_SIGINFO;
    sigfillset(&s.sa_mask); // block all signals during signal handling
    check(sigaction(HANDLED_SIGNAL1, &s, nullptr));
    check(sigaction(HANDLED_SIGNAL2, &s, nullptr));

    check_result(pthread_create(&thread1, nullptr, handling_thread_main, &arg1));
    check_result(pthread_create(&thread2, nullptr, handling_thread_main, &arg2));
    check_result(pthread_create(&thread3, nullptr, blocking_thread_main, &arg3));
    check_result(pthread_create(&thread4, nullptr, blocking_thread_main, &arg4));

    nanosleep(&SIGNAL_INTERVAL, nullptr);
    for(int i = 0; i < NUM_SIGNALS; ++i) {
        send_process_directed_signal(HANDLED_SIGNAL1, i);// may be handled by threads 1-2
        nanosleep(&SIGNAL_INTERVAL, nullptr);
    }
    COUT << std::endl;

    for(int i = 0; i < NUM_SIGNALS; ++i) {
        send_process_directed_signal(HANDLED_SIGNAL2, i);// will be handled by thread 1
        nanosleep(&SIGNAL_INTERVAL, nullptr);
    }
    COUT << std::endl;

    for(int i = 0; i < NUM_SIGNALS; ++i) {
        send_thread_directed_signal(thread1, 1,HANDLED_SIGNAL1, i);// will be handled by thread 1
        nanosleep(&SIGNAL_INTERVAL, nullptr);
    }

    COUT << std::endl;
    for(int i = 0; i < NUM_SIGNALS; ++i) {
        send_process_directed_signal(BLOCKED_SIGNAL, i);// may be taken by threads 3-4
        nanosleep(&SIGNAL_INTERVAL, nullptr);
    }
    COUT << std::endl;

    for(int i = 0; i < NUM_SIGNALS; ++i) {
        send_thread_directed_signal(thread3, 3, BLOCKED_SIGNAL, i);// will be taken by thread 3
        nanosleep(&SIGNAL_INTERVAL, nullptr);
    }
    COUT << std::endl;

    for(int i = 0; i < NUM_SIGNALS; ++i) {
        send_thread_directed_signal(thread1, 1, BLOCKED_SIGNAL, i);// no reaction from thread 1
        nanosleep(&SIGNAL_INTERVAL, nullptr);
    }
    COUT << std::endl;

    sleep(1); // give time to complete the console output

}
