

#include <sys/wait.h>
#include "unistd.h"
#include "sys/resource.h"
#include "stdio.h"
#include "check.hpp"

int main(){

    struct rlimit limit{};
    getrlimit(RLIMIT_NPROC, &limit);
    printf("Parent >> Current RLIMIT_NPROC value:  %ld\n", limit.rlim_cur);
    puts("Parent >> Trying to fork\n");
    auto child_pid = check(fork());
    if(child_pid){
        limit.rlim_cur = limit.rlim_max = 0;

        puts("Child >> Setting process limit to 0");
        check(setrlimit(RLIMIT_NPROC, &limit) );


        getrlimit(RLIMIT_NPROC, &limit);
        printf("Child >> Current RLIMIT_NPROC value:  %ld\n", limit.rlim_cur);

        puts("Child >> Trying to fork");

        auto pid = fork();
        if(pid == 0) exit(0);

        if(pid < 0) puts("Child >> Fork failed\n");
        if(pid > 0) puts("Child >> Fork succeeded\n");

    }
    else{
        puts("Parent >> Waiting for Child");
        wait(NULL);

        getrlimit(RLIMIT_NPROC, &limit);
        printf("Parent >> Current RLIMIT_NPROC value:  %ld\n", limit.rlim_cur);

        puts("Parent >> Trying to fork again");
        auto pid = fork();
        if(pid == 0) exit(0);
        if(pid < 0) puts("Parent >> Fork failed");
        if(pid > 0) puts("Parent >> Fork succeeded");

    }

}