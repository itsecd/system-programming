#include <unistd.h>
#include <fcntl.h>
#include "common.hpp"
#include <wait.h>
#include <sys/mman.h>
#include <memory.h>

inline void disable_zombies(){
    struct sigaction s{};
    s.sa_handler = SIG_IGN;
    check(sigaction(SIGCHLD, &s, nullptr));
}

inline bool process_exists(pid_t pid){
    if(kill(pid, 0) ==0)
        return true; // process exists (either alive or a zombie)
    return errno != ESRCH; // if errno==ESRCH, there is no such process
}


constexpr int MESSAGE_WRITTEN_SIGNAL = SIGUSR1;
constexpr int MESSAGE_READ_SIGNAL = SIGUSR2;

constexpr timespec SIGNAL_WAIT_TIMEOUT {.tv_sec = 1, .tv_nsec = 0};

static bool wait_for_signal(int signal) {
    sigset_t s;
    sigemptyset(&s);
    sigaddset(&s, signal);
    return check_except(sigtimedwait(&s, NULL, &SIGNAL_WAIT_TIMEOUT), EAGAIN) >= 0;
}

void reader(const volatile Message* m, pid_t writer_pid){

    Message local{};
    while(true) {
        while (not wait_for_signal(MESSAGE_WRITTEN_SIGNAL)) {
            if (not process_exists(writer_pid))
                exit(EXIT_SUCCESS);
        }

        memcpy(&local, (const void*)m, sizeof local);
        COUT << local << std::endl;

        check(kill(writer_pid, MESSAGE_READ_SIGNAL));
    }
}


void writer(volatile Message* m, pid_t reader_pid){

    Message local{};

    while(true){
        memset(&local.str, 0, sizeof(local.str)); // clear the string to avoid a possible data leak
        read_message(local);
        memcpy((void*)m, &local, sizeof local); // technically, UB

        check(kill(reader_pid, MESSAGE_WRITTEN_SIGNAL));

        while (not wait_for_signal(MESSAGE_READ_SIGNAL)) {
            if (not process_exists(reader_pid))
                exit(-1);
        }

        if (ask_continue() == false)
            exit(EXIT_SUCCESS);
    }
}


int main(){
    const auto PAGE_SIZE = (size_t)sysconf(_SC_PAGE_SIZE);
    volatile Message* ptr = (Message*)mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0 );
    if (ptr == MAP_FAILED)  // mmap() does not return NULL on failure
        check(NULL);

    sigset_t s;
    sigemptyset(&s);
    sigaddset(&s, SIGUSR1);
    sigaddset(&s, SIGUSR2);
    check(sigprocmask(SIG_BLOCK, &s, NULL));

    disable_zombies();

    int writer_pid = getpid();
    int reader_pid = check(fork());

    if (reader_pid)
        writer(ptr, reader_pid);
    else
        reader(ptr, writer_pid);

}