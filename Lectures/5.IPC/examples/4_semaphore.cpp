#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include <iostream>
#include <sys/wait.h>
#include "check.hpp"

const long long MAX_VALUE = 10000000;

void not_synchronized(volatile long long* x){
    for(long long i = 0; i < MAX_VALUE; ++i)
        ++(*x);
}

void synchronized(volatile long long* x, sem_t* sem){
    for(long long i = 0; i < MAX_VALUE; ++i) {
        check(sem_wait(sem));
        ++(*x);
        sem_post(sem);
    }
}

const char SEM_NAME[] = "/SEM_";
const char SHM_NAME[] = "/SHM2_";
const size_t PAGE_SIZE = 4096;

int main(int argc, char** argv){
    sem_unlink(SEM_NAME);
    sem_t* s = check(sem_open(SEM_NAME, O_CREAT|O_EXCL, S_IRUSR|S_IWUSR, 1));

    shm_unlink(SHM_NAME);
    int fd = check(shm_open(SHM_NAME, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR));
    check(ftruncate(fd, sizeof (long long )));
    volatile long long* ptr = (long long*)check(mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0 ));
    close(fd);

    *ptr = 0;

    pid_t  child_pid = check(fork());
    not_synchronized(ptr);                                    // <=======
    if(!child_pid) exit(0);
    check(waitpid(child_pid, NULL, 0));

    std::cout << "Not synchronized: "<< *ptr << std::endl;

    *ptr = 0;

    child_pid = check(fork());
    synchronized(ptr, s);                                // <=======
    if(!child_pid) exit(0);
    check(waitpid(child_pid, NULL, 0));
    std::cout << "Synchronized: "<< *ptr << std::endl;

}