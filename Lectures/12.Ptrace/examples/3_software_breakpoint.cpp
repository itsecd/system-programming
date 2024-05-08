#include <sys/ptrace.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/debugreg.h>
#include "check.hpp"
#include "cstddef"



void __attribute_noinline__  foo() {
    printf("====Foo==== is called by %d\n", gettid());
}


void tracee(){
    check(ptrace(PTRACE_TRACEME));
    puts("Tracee>> Tracing enabled.Waiting the tracer");
    raise(SIGSTOP);
    puts("Tracee>> Tracer connected");
    foo();
    foo();
}


unsigned int INT3_OPCODE = 0xCC;

long set_breakpoint(int tracee_id, void* address){
    errno = 0;
    auto old_value = ptrace(PTRACE_PEEKDATA,tracee_id, address,0);
    if(old_value < 0 && errno != 0) check(old_value);
    auto new_value = ((old_value >> 8)<<8) |INT3_OPCODE;
    check(ptrace(PTRACE_POKEDATA, tracee_id, address, new_value));
    return old_value;
}

void remove_breakpoint(int tracee_id, void* address, long old_code){
    check(ptrace(PTRACE_POKEDATA, tracee_id, address, old_code));
    user_regs_struct regs{};
    check(ptrace(PTRACE_GETREGS, tracee_id,0, &regs));
    regs.rip -= 1; // rewind back by 1 byte;
    check(ptrace(PTRACE_SETREGS, tracee_id,0, &regs));
}

void tracer(int tracee_id){
    auto address = &foo;
    sleep(1);
    auto old_code = set_breakpoint(tracee_id, (void*)address);
    puts("Tracer>> Breakpoint set");
    while (1) {
        int stat;
        check(waitpid(tracee_id, &stat, WUNTRACED));
        if (WIFSTOPPED(stat)) {
            int sig = WSTOPSIG(stat);
            if (sig == SIGTRAP) {
                siginfo_t info{};
                check(ptrace(PTRACE_GETSIGINFO, tracee_id, 0, &info));
                if(info.si_code == SI_KERNEL) {
                    puts("Tracer>> Breakpoint hit");
                    sleep(1);
                    remove_breakpoint(tracee_id, (void*)address, old_code);
                }
                sig = 0; // do not send SIGTRAP
            }
            check(ptrace(PTRACE_CONT, tracee_id, 0, sig));
        } else if (!WIFCONTINUED(stat)) {
            if(!WIFEXITED(stat))
                puts(strsignal(WTERMSIG(stat)));
            break;
        }
    }
    puts("Tracer>> Tracee is dead");
}

int main(){
    auto tracee_id = check(fork()); // tracee TID is equal to PID of the child process
    if(tracee_id)
        tracer(tracee_id);
    else
        tracee();
}