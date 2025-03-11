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
    timespec t {.tv_sec = 0, .tv_nsec = 100000}; // 10 msec

    while(1) {

        do{
            if (getppid() != writer_pid)
                exit(-1);
            nanosleep(&t, NULL);
        }while(check_except(flock(fd, LOCK_EX|LOCK_NB), EWOULDBLOCK) != 0); // LOCK

        if(!is_file_empty(fd)) {
            Message m{};
            check(lseek(fd, SEEK_SET, 0));
            check(read(fd, &m, sizeof m));
            COUT << m << std::endl;
            check(ftruncate(fd, 0));
        }

        flock(fd, LOCK_UN);                                               // UNLOCK
    }
}


void writer(int fd, pid_t reader_pid){

    timespec t {.tv_sec = 0, .tv_nsec = 10000000};

    Message m{};
    bool read_new_message = true;
    while(1){
        if(read_new_message){
            read_message(m);
            read_new_message = false;
        }

        do {
            if (waitpid(reader_pid, NULL, WNOHANG) != 0)
                exit(-1);
            nanosleep(&t, NULL);
        } while(check_except(flock(fd, LOCK_EX|LOCK_NB), EWOULDBLOCK) != 0); // LOCK

        if(is_file_empty(fd)) {
            read_new_message = true;
            check(lseek(fd, SEEK_SET, 0));
            check(write(fd, &m, sizeof m));
        }
        flock(fd, LOCK_UN);                                                  // UNLOCK

        if(read_new_message)
        {
            nanosleep(&t, nullptr);
            ask_continue();
        }
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