#include <sys/fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

char filename[4097];

int main(){
    bool success = false;
    while(!success) {
        puts("Enter filename");
        scanf("%s", filename);
        puts("Trying to open in read-write mode...");
        int fd = open(filename, O_RDWR);

        if (fd == -1) {
            if (errno == ENOENT) {
                puts("No such file. Try again\n");
            } else {
                perror("Failed to open file");
                exit(2);
            }
        }
        else
            success = true;
    }
    puts("Success!");
}
