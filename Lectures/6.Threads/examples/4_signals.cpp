#include "common.hpp"
#include <pthread.h>
#include <signal.h>
#include <iostream>
#include <string.h>

thread_local volatile sig_atomic_t  last_signal; // not async-signal safe! use with caution
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


void* handling_thread_main(void* arg_){
    last_signal = 0;
    auto arg = (Arg*)arg_;

    while(true) {
        sigsuspend(&arg->mask);
        std::cout << "Thread " << arg->thread_index << " >> " << " handled signal " << std::quoted(strsignal(last_signal))
                << " with value "<< last_value << std::endl;
        timespec t{.tv_sec=0, .tv_nsec=550000000};
        nanosleep(&t, nullptr); //imitate work
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
        int signo = check_except(sigwaitinfo(&wait_mask, &si), EINTR);
        if(signo == -1) {
            //std::cout << "(Thread " << arg->thread_index << " EINTR)"<<std::endl;
            continue;
        }
        std::cout << "Thread " << arg->thread_index << " >> " << " got blocked signal " << std::quoted(strsignal(signo))
                  << " with value "<< si.si_value.sival_int << std::endl;
        timespec t{.tv_sec=0, .tv_nsec=550000000};
        nanosleep(&t, nullptr); //imitate work
    }
}

const int HANDLED_SIGNAL1 = SIGRTMIN; // to thread 1 and 2
const int HANDLED_SIGNAL2 = SIGRTMIN+1; // to thread 2
const int BLOCKED_SIGNAL = SIGRTMIN+3;  // to blocked threads

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

    sigfillset(&arg3.mask) ;
    arg3.wait_signal = BLOCKED_SIGNAL;

    sigfillset(&arg4.mask) ;
    arg4.wait_signal = BLOCKED_SIGNAL;

    struct sigaction s{};
    s.sa_sigaction = handler;
    s.sa_flags = SA_SIGINFO;
    sigfillset(&s.sa_mask); // block all signals during
    sigaction(HANDLED_SIGNAL1, &s, nullptr);
    sigaction(HANDLED_SIGNAL2, &s, nullptr);


    check_result(pthread_create(&thread1, nullptr, handling_thread_main, &arg1));
    check_result(pthread_create(&thread2, nullptr, handling_thread_main, &arg2));
    check_result(pthread_create(&thread3, nullptr, blocking_thread_main, &arg3));
    check_result(pthread_create(&thread4, nullptr, blocking_thread_main, &arg4));

    timespec t{.tv_sec=0, .tv_nsec=500000000};
    nanosleep(&t, nullptr);
    for(int i = 0; i < 3; ++i) {

        sigqueue(getpid(), HANDLED_SIGNAL1, {0}); // may be handled by threads 1-2
        nanosleep(&t, nullptr);
    }
    std::cout << std::endl;
    for(int i = 0; i < 3; ++i) {

        sigqueue(getpid(), HANDLED_SIGNAL2, {1});  // will be handled by thread 1
        nanosleep(&t, nullptr);
    }
    std::cout << std::endl;

    for(int i = 0; i < 3; ++i) {
        pthread_sigqueue(thread1, HANDLED_SIGNAL1, {1}); // will be handled by thread 1
        nanosleep(&t, nullptr);
    }
    std::cout << std::endl;
    for(int i = 0; i < 3; ++i) {
        sigqueue(getpid(), BLOCKED_SIGNAL, {0}); // may be taken by threads 3-4
        nanosleep(&t, nullptr);
    }
    std::cout << std::endl;

    for(int i = 0; i < 3; ++i) {
        pthread_sigqueue(thread3, BLOCKED_SIGNAL, {3}); // will be taken by thread 3
        nanosleep(&t, nullptr);
    }

    for(int i = 0; i < 3; ++i) {
        pthread_sigqueue(thread1, BLOCKED_SIGNAL, {0}); // no reaction from thread 1
        nanosleep(&t, nullptr);
    }
    std::cout << std::endl;


    sleep(1); // give time to complete the console output

}
