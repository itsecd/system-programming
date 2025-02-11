#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "check.hpp"
#include <sys/stat.h>
#include <fcntl.h>

const char DEFAULT_FILENAME[] = "lseek_test_file";

int main(int argc, char* argv[]) {
	const char* filename = DEFAULT_FILENAME;
	printf("Test file name: %s\n", filename);
	unlink(filename);//try to delete file if exists
	int fd = open(filename, O_RDWR | O_CREAT | O_SYNC, S_IRWXU);
	if (fd < 0) {
		perror("Failed to create test file\n");
		exit(-1);
	}

	const char message1[] = "6311";
    puts("Writing to file\n");
	check(write(fd, &message1, sizeof message1 - 1));

	puts("Resetting read/write position to start\n");
	check(lseek(fd, 0, SEEK_SET));

	char buf[1024]{}; //all zero
	ssize_t read_size = check(read(fd, buf, sizeof buf));
	printf("Read %ld bytes from file. Result: %s\n", read_size, buf);

	puts("Resetting file position to start\n");
	check(lseek(fd, 0, SEEK_SET));
	puts("Rewriting file data\n");
	const char message2[] = "6312";
	check(write(fd, &message2, sizeof message2 - 1));

	puts("Resetting file position to start\n");
	check(lseek(fd, 0, SEEK_SET));

	read_size = check(read(fd, buf, sizeof buf));
	printf("Read %ld bytes from file. Result: %s\n", read_size, buf);

	struct stat s {};
	puts("Checking file stats\n");
	check(fstat(fd, &s));
	printf("Current file size: %d\n", (int)s.st_size);
	printf("Used disk blocks: %d\n", (int)s.st_blocks);
	
	printf("Creating a 1MB hole in the file\n");
    const size_t hole_size =1024*1024;
	check(lseek(fd, hole_size, SEEK_CUR));
	ssize_t written_size = check(write(fd, "0", 1));
	if (written_size <= 0)
	{
		printf("Failed to write after the file hole\n");
		exit(-2);
	}

	printf("Reading 4 bytes from the middle of the hole. ");
	check(lseek(fd, hole_size/2, SEEK_SET));
	ssize_t cnt = check(read(fd, &buf, 4));
	if (cnt < 4)
	{
		printf("\nFailed to read 4 bytes from the file hole\n");
		exit(-3);
	}

	bool all_zero = true;
	do { all_zero &= !buf[cnt]; } while (cnt--);

	printf(all_zero ? "All bytes are zeros\n" : "Not all bytes are zeros\n");

	printf("Checking file stats after creating a hole\n");
	check(fstat(fd, &s));
	printf("Current file size: %d\n", (int)s.st_size);
	printf("Used disk blocks: %d\n", (int)s.st_blocks); //The filesystem on WSL does not support holes, so the block count will be equal to (file size)/512
}