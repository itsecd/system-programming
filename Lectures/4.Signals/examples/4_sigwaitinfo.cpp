#include "check.hpp"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "common.h"

int main(){
    sigset_t s;
    sigemptyset(&s);
    sigaddset(&s, SIGUSR1);
    sigaddset(&s, SIGUSR2);
    sigaddset(&s, SIGABRT);
    check(sigprocmask(SIG_BLOCK, &s, NULL));
    puts("Signals are blocked");

    struct sigaction sa{};
    sa.sa_handler = default_handler;
    check(sigaction(SIGUSR1, &sa, NULL));
    check(sigaction(SIGUSR2, &sa, NULL));
    check(sigaction(SIGABRT, &sa, NULL));
    puts("Handlers are set\n");

    check(kill(getpid(), SIGUSR1));
    puts("Sent SIGUSR1 to self");
    check(kill(getpid(), SIGABRT));
    puts("Sent SIGABRT to self");
    check(kill(getpid(), SIGUSR2));
    check(kill(getpid(), SIGUSR2));
    puts("Sent SIGUSR2 to self twice\n");


    sigemptyset(&s);
    sigaddset(&s, SIGUSR1);
    puts("Calling sigwaitinfo() with SIGUSR1 in sigset");
    int sig = check(sigwaitinfo(&s, nullptr));      //get SIGUSR1
    printf("Sigwaitinfo() returned signal %d  (%s)\n\n", sig, signal_name(sig));

    sigemptyset(&s);
    sigaddset(&s, SIGABRT); // only SIGABRT is blocked
    puts("Calling sigsuspend() with SIGUSR2 unblocked");
    check_except(sigsuspend(&s), EINTR); //unblock SIGUSR1, wait for one;
    puts("After sigsuspend()\n");

    sigemptyset(&s);
    puts("Calling sigprocmask() to unblock all signals");
    check(sigprocmask(SIG_SETMASK, &s, nullptr));
    puts("After sigprocmask()");

}
