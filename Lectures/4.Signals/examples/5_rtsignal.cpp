#include "check.hpp"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "common.h"

volatile sig_atomic_t  last_value = 0;
volatile sig_atomic_t last_signal = 0;


void rt_handler(int sig, siginfo_t* info, void*){
    printf("----extended handler >> Got signal %d (%s) with value %d\n", sig,signal_name(sig), info->si_value.sival_int); // << DO THIS WITH CAUTION
}

int main(){

    sigset_t s;
    sigemptyset(&s);
    sigaddset(&s, SIGUSR1);
    sigaddset(&s, SIGRTMIN);
    check(sigprocmask(SIG_SETMASK, &s, NULL)); //block signals
    puts("Blocked SIGUSR1 and SIGRTMIN");

    struct sigaction sa{};
    sa.sa_handler = default_handler;
    sa.sa_mask = s;
    check(sigaction(SIGUSR1, &sa, NULL));

    sa.sa_sigaction = rt_handler;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_mask = s;
    check(sigaction(SIGRTMIN, &sa, NULL));
    puts("Handlers are set\n");

    check(kill(getpid(), SIGUSR1));
    check(kill(getpid(), SIGUSR1));
    puts("Sent SIGUSR1 to self twice");
    check(sigqueue(getpid(), SIGRTMIN, sigval_t {6311}));
    check(sigqueue(getpid(), SIGRTMIN, sigval_t {6312}));
    puts("Sent SIGRTMIN to self twice\n");


    sigemptyset(&s);
    puts("Calling sigprocmask() to unblock all signals");
    check(sigprocmask(SIG_SETMASK, &s, nullptr));
    puts("After sigprocmask()");


}
