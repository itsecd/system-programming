#include <unistd.h>
#include <sys/file.h>
#include <string.h>
#include <cstdio>
#include "check.hpp"

const auto FILENAME = "/tmp/file_to_lock.txt";

int main() {
	puts("This program can have only a single instance at the time");

	int fd = check(open(FILENAME, O_RDWR | O_CREAT, S_IRWXU));
	
	int result = flock(fd, LOCK_EX | LOCK_NB); //get the lock, do not block if fail
	if (result) {
		if (errno == EAGAIN) {
            char buf[33]{};
            check(read(fd, buf, 32));
			fprintf(stderr, "Failed to get the file lock (held by process %s). Exiting\n", buf);
			exit(-1);
		}
		check(result);// if not EAGAIN, kill the process as usual
	}
    else {
        char buf[33]{}; // enough for a number
        snprintf(buf, 32, "%d", getpid());
        check(ftruncate(fd, 0));
        check(write(fd, buf, strlen(buf)));
    }
	puts("Enter anything to quit");
	getchar();
}
