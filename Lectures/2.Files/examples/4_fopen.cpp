#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <fstream>
#include "check.hpp"
#include <cstring>

const char DEFAULT_FILENAME[] = "fopen_test_file";
const char MESSAGE[] = "You probably will not see this\n";


void sudden_exit() {
	printf("Killing process\n");
	abort();
}

void cpp(const char* filename) {
	std::fstream f{ filename, std::fstream::trunc };
	f.exceptions(std::fstream::failbit|std::fstream::badbit);//throw if failed to open the file
	f << MESSAGE;
    sudden_exit();
}

void c(const char* filename) {
	auto f = check(fopen(filename, "w"));
	check(fputs(MESSAGE, f));
    sudden_exit();
}

void syscalls(const char* filename) {
	int fd = check(open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU));
	check(write(fd, MESSAGE, sizeof MESSAGE - 1));
    sudden_exit();
}

/*
*  C and C++ use internal buffers to speed up the I/O operations
*  This however can lead to a data loss if the process unexpectedly dies (by a signal, for example)
*  This can be avoided by directly using  the syscalls (sacrificing some performance and convenience)
*/

char cwdbuffer[4096];

int main(int argc, char* argv[]) {
	const char* filename = DEFAULT_FILENAME;
	if (argc > 2) {
		filename = argv[2];
	}

    bool is_abs_path = filename[0] == '/';
    if(!is_abs_path) // not an absolute path;
        getcwd(cwdbuffer, sizeof cwdbuffer);
	printf("Test file name: %s%c%s\n", cwdbuffer, is_abs_path?' ':'/', filename);
	printf("Check the content after this program exits\n");

	int choice = 0;
	if (argc > 1)
	{
		if (!strcmp(argv[1], "c"))
			choice = 1;
		else if (!strcmp(argv[1], "c++"))
			choice = 2;
	}
	switch (choice)
	{
	case 1:
		printf("Using C I/O\n");
		c(filename);
	case 2:
		printf("Using C++ I/O\n");
		cpp(filename);
	default:
		printf("Using UNIX syscalls for I/O. Pass 'c' or 'c++' as a first argument to see what happens\n");
		syscalls(filename);
	}
}