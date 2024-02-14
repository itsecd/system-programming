#include <stdlib.h>
#include <stdio.h>
#include <string.h>
extern char** environ; /*see  'man environ' */


void print_args(int argc, char* argv[]) {
	printf("Arguments count: %d", argc);
    if(!argc) return;
	puts("\nArguments:\n");
	for (int i = 0; i < argc; ++i)
		printf("%s\n", argv[i]);
}

void print_env() {
	char** v = environ;
	puts("\nEnvironment variables:\n");
    if (!*v)
        puts("None");
	while (*v)
	{
		printf("%s\n", *v);
		++v;
	}
}



int main(int argc, char* argv[]) {
	print_args(argc, argv);
    bool do_print_env = false;
    for(int i = 0; i < argc && ! do_print_env; ++i)
        if(strcmp(argv[i],  "--print-env") == 0)
            do_print_env = true;
    if(do_print_env)
	    print_env();
	auto x_var = getenv("X");
	if (!x_var)
		puts("\nThe variable 'X' was not defined\n");
	else
		printf("\nValue of the variable 'X' was '%s'\n", x_var);
	setenv("X", "6311_6312", 1);
	x_var = getenv("X");
	printf("Value of the 'X' variable now is '%s'\n", x_var);

    if(do_print_env)
	    print_env();
}


