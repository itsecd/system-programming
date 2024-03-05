#include "check.hpp"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

const char msg[] = "-------- handler\n";

void handler(int){
    write(STDOUT_FILENO, msg, sizeof msg -1);
}

int main(){

    sigset_t s;
    sigemptyset(&s);
    sigaddset(&s, SIGUSR1);
    sigaddset(&s, SIGUSR2);
    check(sigprocmask(SIG_SETMASK, &s, NULL));

    struct sigaction sa{};
    sa.sa_handler = handler;
    check(sigaction(SIGUSR1, &sa, NULL));
    check(sigaction(SIGUSR2, &sa, NULL));

    check(kill(getpid(), SIGUSR1));
    puts("Sent SIGUSR1 to self");
    check(kill(getpid(), SIGUSR2));
    check(kill(getpid(), SIGUSR2));
    puts("Sent SIGUSR2 to self twice");

    sigemptyset(&s);
    sigaddset(&s, SIGUSR1);
    int sig = check(sigwaitinfo(&s, nullptr));      //get SIGUSR1
    printf("Sigwaitinfo returned signal %d  (%s)\n", sig, strsignal(sig));

    sigemptyset(&s);
    check_except(sigsuspend(&s), EINTR); //unblock all signals, wait for one;
    puts("After sigsuspend()");
    check(sigprocmask(SIG_SETMASK, &s, nullptr)); //unblock all signal
    puts("After sigprocmask()");

}
