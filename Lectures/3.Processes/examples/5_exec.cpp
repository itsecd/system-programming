#include "check.hpp"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    if(argc < 2) {
        fputs("Not enough args", stderr);
        exit(-1);
    }

    auto path = argv[1];

	printf("Currently I am Example, but soon I will become %s\n", path);
    char** args = new char*[argc];
    args[0] = path;
    memcpy( &args[1],&argv[2], (argc-2)*sizeof(char*)); // copy arguments from 2nd to last
    args[argc-1] = nullptr;
	check(execve(path, args, environ));

	printf("You will never see this line :)\n");
}