#include <unistd.h>
#include <stdio.h>
#include "check.hpp"
#include <fcntl.h>
const char FILENAME[] = "dup_test";


int main(int argc, char* argv[]) {
	int stdout_copy_fd = check(dup(STDOUT_FILENO));
	printf("The stdout descriptor was duplicated to the descriptor %d\n", stdout_copy_fd);
	puts("Closing the stdout descriptor\n");
	close(STDOUT_FILENO);
	const char msg[] = "   you see this   \n";
	check(write(stdout_copy_fd, msg, sizeof msg -1));
	puts("    but you will not see this because descriptor 1 (stdout) was closed\n"); //!!!
	check(dup2(stdout_copy_fd, STDOUT_FILENO));
	puts("\nStdout descriptor is restored\n\n");


	//int fd = check(mkstemp("XXXXXX")); //see man
	int fd = check(open(FILENAME, O_WRONLY|O_CREAT, S_IRWXU)); 
	int fd2 = check(dup2(fd, 10)); //you can use 2 to retarget STDOUT to the file

	printf("Opened %s and duplicated the descriptor\n", FILENAME);
	printf("Position of the 1st descriptor:%ld\n", check(lseek(fd, 0, SEEK_CUR)));
	printf("Position of the 2nd descriptor:%ld\n", check(lseek(fd2, 0, SEEK_CUR)));

	puts("Write 20 bytes through the 1st descriptor");
	check(write(fd, msg, sizeof msg));
	printf("Position of the 1st descriptor:%ld\n", check(lseek(fd, 0, SEEK_CUR)));
	printf("Position of the 2nd descriptor:%ld\n", check(lseek(fd2, 0, SEEK_CUR)));

	close(fd);
	close(fd2);


	fd = check(open(FILENAME, O_WRONLY)); //see man
	fd2 = check(open(FILENAME, O_WRONLY)); //see man

	printf("\nOpened %s twice\n", FILENAME);
	printf("Position of the 1st descriptor:%ld\n", check(lseek(fd, 0, SEEK_CUR)));
	printf("Position of the 2nd descriptor:%ld\n", check(lseek(fd2, 0, SEEK_CUR)));

	printf("Write 20 bytes through the 1st descriptor\n");
	check(write(fd, msg, sizeof msg));
	printf("Position of the 1st descriptor:%ld\n", check(lseek(fd, 0, SEEK_CUR)));
	printf("Position of the 2nd descriptor:%ld\n", check(lseek(fd2, 0, SEEK_CUR)));
}