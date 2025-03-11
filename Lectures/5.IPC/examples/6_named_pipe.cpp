#include <unistd.h>
#include <fcntl.h>
#include "common.hpp"
#include <sys/stat.h>


void reader(int fd){
    while(true) {
        Message m{};
        ssize_t read_size = check(read(fd, &m, sizeof m));
        if(read_size == 0) exit(0);
        COUT << m << std::endl;
    }
}


void writer(int fd){
    while(true){
        Message m{};
        read_message(m);

        check(write(fd, &m, sizeof m));  // may fail with EPIPE

        struct timespec t{.tv_sec=0, .tv_nsec=100000};
        nanosleep(&t, nullptr);
        ask_continue();
    }
}


const char FIFO_NAME[]= "/tmp/FIFO_";
int main(int argc, char** argv){
    unlink(FIFO_NAME); // delete if exists
    check(mkfifo(FIFO_NAME, S_IRUSR|S_IWUSR));

    int reader_pid = check(fork());

    if (reader_pid) {
        int write_fd = check(open(FIFO_NAME, O_WRONLY));
        writer(write_fd);
    }
    else {
        int read_fd = check(open(FIFO_NAME, O_RDONLY));
        reader(read_fd);
    }

}