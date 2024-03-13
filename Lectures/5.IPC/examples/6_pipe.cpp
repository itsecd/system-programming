#include <unistd.h>
#include <fcntl.h>
#include "common.hpp"
#include <iostream>


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

        check(write(fd, &m, sizeof m));

        struct timespec t{.tv_sec=0, .tv_nsec=100000};
        nanosleep(&t, nullptr);
        ask_continue();
    }
}


int main(int argc, char** argv){

    int pipe_fd[2];
    check(pipe(pipe_fd));


    auto[read_fd, write_fd] = pipe_fd;

    int reader_pid = check(fork());

    if (reader_pid) {
        close(read_fd);
        writer(write_fd);
    }
    else {
        close(write_fd);
        reader(read_fd);
    }

}