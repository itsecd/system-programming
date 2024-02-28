#include "check.hpp"
#include <unistd.h>

int main() {
	auto id = check(fork());

	if (id > 0) { /*id > 0, we are parent process*/
		printf("%d>> I am your father!\n", getpid());
	}
	else {      /*id == 0, we are child process*/
		printf("%d>> Noooooooooooooooooooo!\n", getpid());
	}
}

