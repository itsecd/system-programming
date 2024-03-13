#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "common.hpp"
#include <wait.h>
#include <iostream>
#include <sys/mman.h>
#include <memory.h>

void reader(const volatile Message* m, pid_t writer_pid){
    sigset_t s;
    sigemptyset(&s);
    sigaddset(&s, SIGUSR1);

    timespec t {.tv_sec = 1, .tv_nsec = 0};

    while(1) {
        while (check_except(sigtimedwait(&s, NULL, &t), EAGAIN) < 0) {
            if (getppid() != writer_pid)
                exit(-1);
        }

        Message local{m->x};
        memcpy(&local.str, (const void*)m->str, sizeof local.str);

        COUT << local << std::endl;
        kill(writer_pid, SIGUSR2);
    }
}


void writer(volatile Message* m, pid_t reader_pid){
    sigset_t s;
    sigemptyset(&s);
    sigaddset(&s, SIGUSR2);

    timespec t {.tv_sec = 0, .tv_nsec = 100000};
    while(1){
        Message local{};
        read_message(local);
        m->x = local.x;
        memcpy((void *) m->str, local.str, sizeof local.str); // technically, UB

        kill(reader_pid, SIGUSR1);

        while (check_except(sigtimedwait(&s, NULL, &t), EAGAIN) < 0) {
            if (waitpid(reader_pid, 0, WNOHANG) != 0)
                exit(-1);
        }

        ask_continue();
    }
}

constexpr size_t PAGE_SIZE = 4096;

const char SHM_NAME[] = "/SHM_";

int main(){
    shm_unlink(SHM_NAME);
    int fd = check(shm_open(SHM_NAME, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR));
    check(ftruncate(fd, sizeof (Message)));
    volatile Message* ptr = (Message*)check(mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0 ));
    close(fd);

    sigset_t s;
    sigemptyset(&s);
    sigaddset(&s, SIGUSR1);
    sigaddset(&s, SIGUSR2);
    check(sigprocmask(SIG_BLOCK, &s, NULL));


    int writer_pid = getpid();
    int reader_pid = check(fork());

    if (reader_pid)
        writer(ptr, reader_pid);
    else
        reader(ptr, writer_pid);

}