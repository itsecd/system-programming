#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <wait.h>
#include "check.hpp"

void child(){
    while(1){
        puts("Child >> I'm alive");
        sleep(1);
    }
}

void parent(int child_pid){
    sleep(3);
    puts("Parent >> Senging SIGSTOP");
    check(kill(child_pid, SIGSTOP));
    sleep(3);
    puts("Parent >> Senging SIGCONT");
    check(kill(child_pid, SIGCONT));
    sleep(3);
    puts("Parent >> Senging SIGINT");
    check(kill(child_pid, SIGINT));
    sleep(1);

    int status;
    wait(&status);
    printf("Was child process killed? %s. Signal description: %s, (signal %d)",
           WIFSIGNALED(status) ? "Yes" : "No",
           strsignal(WTERMSIG(status)), WTERMSIG(status));

}



int main(){
    int child_pid = check(fork());
    if(child_pid)
        parent(child_pid);
    else
        child();
}