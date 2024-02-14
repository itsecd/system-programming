#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

void print_help_and_exit(const char* error) {
	if (error)
		printf("%s\n", error);
	printf("Usage: copy_file FROM TO\n");
	exit(1);
}


int main(int argc, char* argv[]) {
	if (argc < 3) {
		print_help_and_exit("Too few arguments");
	}
	if (argc > 3) {
		print_help_and_exit("Too many arguments");
	}

	const char  *from = argv[1],
				*to = argv[2];


	int from_fd = open(from, O_RDONLY);
	if (from_fd < 0){
		perror("Failed to open the source file "); 
		exit(2);
	}

	int to_fd = open(to, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU); //create file if not exists, else truncate
	if (to_fd < 0) {
		perror("Failed to create the destination file ");
		exit(3);
	}

	char buffer[1024]{};

    ssize_t read_count{};
	do {
		read_count = read(from_fd, &buffer, sizeof(buffer));
		if (read_count < 0) {
			perror("Error when reading the source file");
			_exit(4);
		}
		if (read_count > 0)
		{
			ssize_t written_count = write(to_fd, &buffer, read_count);
			if (written_count < 0)
			{
				perror("Error when writing the destination file");
				_exit(5);
			}
		}
	} while(read_count > 0);

}