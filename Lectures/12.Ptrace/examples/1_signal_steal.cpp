#include <signal.h>
#include <sys/ptrace.h>
#include <string.h>
#include <sys/wait.h>
#include "check.hpp"

void tracer(pid_t tracee_id){
    sleep(1);
    check(ptrace(PTRACE_CONT, tracee_id, 0, 0));
    while(1){
        int status;
        check(waitpid(tracee_id, &status, WUNTRACED));

        int  sig = 0;
        if(WIFSTOPPED(status)){
            sig = WSTOPSIG(status);
            printf("Tracer>> Tracee got a signal %s\n", strsignal(sig));
            if(sig == SIGUSR1) {
                puts("Tracer>> SIGUSR1 is not allowed, suppressing");
                sig = 0;
            }
        }
        else if (!WIFCONTINUED(status))
            break;
        check(ptrace(PTRACE_CONT, tracee_id, 0, sig));
    }
    puts("Tracer>> Tracee is dead");
}

void handler(int signal){
    printf("Got signal %s\n", strsignal(signal));
}

void tracee(){
    signal(SIGUSR1, handler);  //  too lazy to call sigaction :)
    signal(SIGUSR2, handler);
    check(ptrace(PTRACE_TRACEME));
    puts("Tracee>> Tracing enabled.Waiting the tracer");
    raise(SIGSTOP);
    puts("Tracee>> Tracer connected");
    puts("Tracee>> Sending self SIGUSR1");
    raise(SIGUSR1);
    puts("Tracee>> Sending self SIGUSR2");
    raise(SIGUSR2);
}


int main(){
    auto tracee_id = check(fork()); // tracee TID is equal to PID of the child process
    if(tracee_id)
        tracer(tracee_id);
    else
        tracee();
}