#include "check.hpp"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

const char msg[] = "---- SIGFPE handler\n";

void handler(int sig){
    write(STDOUT_FILENO, msg, sizeof msg -1 );
    _exit(-1);
}

int main(int argc, char**){
    struct sigaction s{};
    s.sa_handler = handler;
    if(argc > 1) {
        check(sigaction(SIGFPE, &s, NULL));
    }

    volatile int x= 0;
    printf("%d\n", 5/x);

}