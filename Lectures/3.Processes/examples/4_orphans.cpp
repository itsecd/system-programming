#include "check.hpp"
#include <unistd.h>

int main(){
    pid_t parent_pid = getpid(); // remember the parent pid _before_ fork

    pid_t pid = check(fork());

    if(pid == 0){
        printf("Child >> I'm a child of %d\n", parent_pid);
        bool not_orphan = true;
        while(not_orphan){
            not_orphan = getppid() == parent_pid;
            if(not_orphan){
                puts("Child >> I'm not an orphan!");
            }
            sleep(1);
        }
        puts("Child >> I'm an orphan! (╥﹏╥)");
        printf("Child >> I'm was adopted by %d\n", getppid());
    }
    else{
        sleep(2);
        puts("Parent >> Enter anything to end me...");
        getchar();
        exit(0);
    }
}