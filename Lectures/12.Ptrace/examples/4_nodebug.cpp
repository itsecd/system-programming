#include <sys/ptrace.h>
#include "check.hpp"

int main(){
    if(ptrace(PTRACE_TRACEME) < 0)
        puts("Please, do not debug me :(");
    else
        puts("Hello, world!");
}