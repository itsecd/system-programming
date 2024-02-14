#include <stdio.h>
#include <unistd.h>
#include "check.hpp"



int main(int argc, char* argv[]) {
	printf("Max number of opened files: %ld\n", sysconf(_SC_OPEN_MAX));
	printf("Max arguments count: %ld\n", sysconf(_SC_ARG_MAX));

    long path_max = pathconf("/", _PC_PATH_MAX);
    printf("Max absolute path length: %ld\n", path_max);

    char* buffer = (char*)malloc(path_max+1);

	check(getcwd(buffer, path_max+1));
	printf("Current directory %s:\n", buffer);
	printf("Max relative path length for the current directory: %ld\n", pathconf(buffer, _PC_PATH_MAX));
	printf("Max filename length in the filesystem of the current directory: %ld\n", pathconf(buffer, _PC_NAME_MAX));

}