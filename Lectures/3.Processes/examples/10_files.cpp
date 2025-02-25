#include "check.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

const char FILENAME[] = "10_files.txt";

int main(){

    int fd  = check(open(FILENAME, O_CREAT|O_TRUNC|O_RDWR, S_IRUSR|S_IWUSR));

    check(write(fd, "aaaa", 4));

    if( check(fork()) ){
        auto pos = check(lseek(fd, 0, SEEK_CUR));
        printf("Parent:\t current r/w position: %ld\n", pos);
        sleep(2);
        pos = check(lseek(fd, 0, SEEK_CUR));
        printf("Parent:\t current r/w position: %ld\n",  pos);
    }
    else{
        sleep(1);
        auto pos = check(lseek(fd, 0, SEEK_CUR));
        printf("Child:\t current r/w position: %ld\n", pos);
        check(write(fd, "bbbb", 4));
        pos = check(lseek(fd, 0, SEEK_CUR));
        printf("Child:\t Wrote 4 bytes. New r/w position: %ld\n",  pos);
    }

}
