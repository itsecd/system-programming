#include "check.hpp"
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "common.h"

int main(int argc, char**){
    struct sigaction s{};
    s.sa_handler = default_handler;
    check(sigaction(SIGABRT, &s, NULL));
    puts("SIGABRT handler is set\n");

    for(int i = 0; i < 3; ++i) {
        puts("Sending SIGABRT to self");
        kill(getpid(), SIGABRT);
    }

    puts("\nCalling abort()");
    abort();
    puts("you won't see this");
}