#include "check.hpp"
#include <unistd.h>
#include <signal.h>
#include <time.h>
void handler(int sig){}


void print_time(){
    auto abs_time = time(NULL);
    auto current_time = localtime(&abs_time);
    printf("Current time: %s", asctime(current_time));
}

int main(){
    struct sigaction sa{};
    sa.sa_handler = handler;
    check(sigaction(SIGALRM, &sa, NULL)); // setting handler so SIGALRM won't kill the process

    print_time();
    puts("Setting alarm 3 seconds ahead");
    alarm(3);
    puts("Trying to sleep 10 seconds");
    struct timespec t{.tv_sec = 10, .tv_nsec = 0}, left{};

    int result = nanosleep(&t, &left);// can not use sleep() with alarm()
    print_time();
    if(result != 0 && errno == EINTR)
        puts("Sleep was interrupted by a signal");
    printf("Remaining sleep time: %ld.%ld", left.tv_sec, left.tv_nsec / 1000000);

}
