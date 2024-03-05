#include "check.hpp"
#include <signal.h>
#include <unistd.h>
#include <string.h>


const char msg[] = "---- SIGABRT handler\n";

void handler(int sig){
    write(STDOUT_FILENO, msg, sizeof msg -1 );
}

int main(int argc, char**){
    struct sigaction s{};
    s.sa_handler = handler;
    check(sigaction(SIGABRT, &s, NULL));

    kill(getpid(), SIGABRT);
    kill(getpid(), SIGABRT);
    kill(getpid(), SIGABRT);

    abort();
}