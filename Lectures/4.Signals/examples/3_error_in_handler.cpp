#include <unistd.h>
#include <signal.h>
#include "check.hpp"
const bool DO_WRITE = false;
volatile long long value = 0;


const char msg1[] = "Parent\n";
const char msg2[] = "Child \n";

void handler(int sig){
    value = -1;
}

int main(){
    struct sigaction s{};
    s.sa_handler = handler;
    check(sigaction(SIGUSR1, &s, NULL));

    auto ppid = getpid();
    auto pid = check(fork());
    while(1){
        if(pid){
            auto local_copy = value;
            if(local_copy != -1 && local_copy != 0)
            {
                break;
            }
            value = 0;
            if (DO_WRITE)
                write(1, msg1, sizeof msg1 -1);
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