#include "check.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>


int spawn_child(bool use_normal_exit) {
	pid_t id = check(fork());
	if (id)		// id!= 0,
		return id;	//we are in the parent process, return child's PID
	printf("Child %d >> I'm born!\n", getpid());

	sleep(2+!use_normal_exit); //simulate some work for 2-3 sec
	
	printf("Child %d >> I'm done!\n", getpid());
    if(use_normal_exit)
	    exit(10); //Stop execution here, don't return into main()
    else
        abort();
}

int main() {
	pid_t child1_pid = spawn_child(true);
	pid_t child2_pid = spawn_child( false);

	sleep(1); // wait a little bit

	int stat;
	waitpid(child1_pid, &stat, 0);
	printf("Child %d has exited? %s, exit code is %d\n"
           , child1_pid
           , WIFEXITED(stat) ? "Yes" : "No"
           , WEXITSTATUS(stat));

	waitpid(child2_pid, &stat, 0);
	printf("Child %d was teminated? %s, process was %s (signal %d)\n"
           , child2_pid
           , WIFSIGNALED(stat) ? "Yes" : "No"
           , strsignal(WTERMSIG(stat))
           , WTERMSIG(stat)); // strsignal gives the signal description,
}