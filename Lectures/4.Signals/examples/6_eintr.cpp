#include "check.hpp"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <wait.h>
#include "common.h"

void child(int parent_pid){
    sleep(1);
    check(sigqueue(parent_pid, SIGRTMIN, {0}));
    sleep(1);
    check(sigqueue(parent_pid, SIGRTMIN, {0}));
    sleep(1);
}

void parent(){

    sigset_t  s{};
    sigemptyset(&s);
    sigaddset(&s, SIGUSR1);
    struct timespec t {.tv_sec =1, .tv_nsec = 500000000}; // 1.5 s

    puts("Calling sigtimedwait()");
    int res = check_except(sigtimedwait(&s, nullptr, &t), EINTR, EAGAIN);
    if(res >= 0)
        puts("Sigtimedwait completed"); // never happen
    else if(errno == EINTR)
        puts("Sigtimedwait interrupted by unexpected signal");
    else
        puts("Sigtimedwait timed out");

    puts("\nCalling wait()");
    res = check_except(wait(NULL), EINTR);
    if (res < 0)
        puts("Wait was interrupted");
    else
        puts("Wait completed");
}

int main(int argc, char** argv){
    struct sigaction s{};
    s.sa_handler = default_handler;
    if(argc > 1)
        s.sa_flags = SA_RESTART;
    check(sigaction(SIGRTMIN, &s, NULL));

    auto parent_pid = getpid();
    if(check(fork())){
        parent();
    }
    else{
        child(parent_pid);
    }

}

