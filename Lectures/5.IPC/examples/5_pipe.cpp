#include <unistd.h>
#include <fcntl.h>
#include "common.hpp"
#include <iostream>


void reader(int fd){
    while(true) {
        Message m{};
        ssize_t read_size = check(read(fd, &m, sizeof m));
        if(read_size == 0) exit(0); // read() can return 0 only if a "line break" occured
        COUT << m << std::endl;
    }
}


void writer(int fd){
    while(true){
        Message m{};
        read_message(m);

        check(write(fd, &m, sizeof m)); // if a "line break" is detected, the process will die here, no action needed

        struct timespec t{.tv_sec=0, .tv_nsec=100000};
        nanosleep(&t, nullptr); // small delay to give some time for the reader to print the message

        if (not ask_continue())
            exit(EXIT_SUCCESS);
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