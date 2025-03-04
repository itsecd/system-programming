#include <unistd.h>
#include <signal.h>
#include "check.hpp"
const bool DO_WRITE = false;
volatile long long value = 0;


const char msg1[] = "Parent\n";
const char msg2[] = "Child \n";

void* malloc_or_free(void* p){
    if(!p)
        return malloc(64);
    else {
        free(p);
        return nullptr;
    }
}

void handler(int sig){
    static volatile void* volatile p = nullptr; //disable optimization
    p = malloc_or_free((void*)p);
}

int main(){
    struct sigaction s{};
    s.sa_handler = handler;
    check(sigaction(SIGUSR1, &s, NULL));

    auto ppid = getpid();
    auto pid = check(fork());
    while(1){
        if(pid){
            static volatile void* volatile p = nullptr; //disable optimization
            p = malloc_or_free((void*)p);
        }
        else{
            kill(ppid, SIGUSR1);
            if(getppid() != ppid)
                break;
            if(DO_WRITE)
                write(1, msg2, sizeof msg2 -1);
        }
    }

}