#include <unistd.h>
#include <fcntl.h>
#include "common.hpp"
#include <wait.h>
#include <sys/stat.h>
#include <sys/file.h>

bool is_file_empty(int fd){
    struct stat s;
    check(fstat(fd, &s));
    return s.st_size == 0;
}

void reader(int fd, pid_t writer_pid){
    timespec t {.tv_sec = 0, .tv_nsec = 10000000}; // 10 msec

    while(true) {
        //get the lock
        // get the file lock
        bool has_file_lock = false;
        do {
            has_file_lock = check_except(flock(fd, LOCK_EX|LOCK_NB), EWOULDBLOCK) == 0;
            if (not is_parent_alive(writer_pid))
                exit(EXIT_SUCCESS);
            if (not has_file_lock)
                nanosleep(&t, NULL); // small delay to slow down the cycle
        } while(not has_file_lock);

        //if there is an unread message - read and process
        if(!is_file_empty(fd)) {
            Message m{};
            check(lseek(fd, SEEK_SET, 0));
            check(read(fd, &m, sizeof m));
            COUT << m << std::endl;
            check(ftruncate(fd, 0));
        }

        //release the lock
        flock(fd, LOCK_UN);                                               // UNLOCK
    }
}

void writer(int fd, pid_t reader_pid){

    timespec t {.tv_sec = 0, .tv_nsec = 10000000};
    Message m{};

    while (true) {
        read_message(m);

        bool message_written = false;
        do {
            // get the file lock
            bool has_file_lock = false;
            do {
                has_file_lock = check_except(flock(fd, LOCK_EX|LOCK_NB), EWOULDBLOCK) == 0;
                if (not is_child_alive(reader_pid))
                    exit(EXIT_FAILURE);
                if (not has_file_lock)
                    nanosleep(&t, NULL); // small delay to slow down the cycle
            } while(not has_file_lock);

            // check if a previous message was written
            if (is_file_empty(fd)) {
                //if yes - write a new message
                check(lseek(fd, SEEK_SET, 0));
                check(write(fd, &m, sizeof m));
                message_written = true;
            }

            flock(fd, LOCK_UN); // don't forget to release the lock!
        }while (not message_written);

        if (!ask_continue())
            exit(EXIT_SUCCESS);
    }
}


int main(){
    char filename[] = "/tmp/FILE_XXXXXX";
    int fd = mkstemp(filename);

    COUT << "Opened file "  << filename << std::endl;

    int writer_pid = getpid();
    int reader_pid = check(fork());

    if (reader_pid)
        writer(fd, reader_pid);
    else
        reader(fd, writer_pid);
}