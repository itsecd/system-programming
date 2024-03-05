#include "check.hpp"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


volatile sig_atomic_t  last_value = 0;
volatile sig_atomic_t last_signal = 0;

void handler(int sig){
    printf("Handler >> Got signal %d\n", sig); // << DO THIS WITH  CAUTION
    last_signal = sig;
}

void handler2(int sig, siginfo_t* info, void*){
    printf("Extended handler >> Got signal %d with value %d\n", sig, info->si_value.sival_int); // << DO THIS WITH  CAUTION
}


void get_one_signal(){
    sigset_t s;
    sigfillset(&s);
    siginfo_t  info;
    int sig = sigwaitinfo(&s, &info );
    printf("Got signal %d with value %d\n", sig, info.si_value.sival_int);
}

int main(){

    sigset_t s;
    sigemptyset(&s);
    sigaddset(&s, SIGUSR1);
    sigaddset(&s, SIGRTMIN);
    check(sigprocmask(SIG_SETMASK, &s, NULL)); //block signals

    struct sigaction sa{};
    sa.sa_handler = handler;
    sa.sa_mask = s;
    check(sigaction(SIGUSR1, &sa, NULL));

    sa.sa_sigaction = handler2;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_mask = s;
    check(sigaction(SIGRTMIN, &sa, NULL));

    check(kill(getpid(), SIGUSR1));
    check(kill(getpid(), SIGUSR1));
    puts("Sent SIGUSR1 to self twice");
    check(sigqueue(getpid(), SIGRTMIN, sigval_t {6311}));
    check(sigqueue(getpid(), SIGRTMIN, sigval_t {6312}));
    puts("Sent SIGRTMIN to self twice");

    sigemptyset(&s);
    check(sigprocmask(SIG_SETMASK, &s, NULL)); // unblock all signals
    sleep(1);

}
