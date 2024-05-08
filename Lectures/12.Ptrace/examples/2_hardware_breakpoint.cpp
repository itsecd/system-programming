#include <sys/ptrace.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/debugreg.h>
#include "check.hpp"
#include "cstddef"

volatile unsigned int value=0x63116312;

void tracee(){
    check(ptrace(PTRACE_TRACEME));
    puts("Tracee>> Tracing enabled.Waiting the tracer");
    raise(SIGSTOP);
    puts("Tracee>> Tracer connected");

    printf("Tracee>> Value was 0x%x\n", value);
    value = 0x63126313;
    printf("Tracee>> Value now is 0x%x\n", value);
}

union dr7_t {
    struct {
        unsigned int dr0_local: 1;
        unsigned int dr0_global: 1;
        unsigned int dr1_local: 1;
        unsigned int dr1_global: 1;
        unsigned int dr2_local: 1;
        unsigned int dr2_global: 1;
        unsigned int dr3_local: 1;
        unsigned int dr3_global: 1;
        unsigned int le: 1;
        unsigned int ge: 1;
        unsigned int reserved_10: 1;
        unsigned int rtm: 1;
        unsigned int reserved_12: 1;
        unsigned int gd: 1;
        unsigned int reserved_14_15: 2;
        unsigned int dr0_break: 2;
        unsigned int dr0_len: 2;
        unsigned int dr1_break: 2;
        unsigned int dr1_len: 2;
        unsigned int dr2_break: 2;
        unsigned int dr2_len: 2;
        unsigned int dr3_break: 2;
        unsigned int dr3_len: 2;
    } fields;
    unsigned long long value;
};

void setup_breakpoint(pid_t tracee_id){
    auto dr0_offset = offsetof(user, u_debugreg[DR_FIRSTADDR] );

    auto dr7_offset = offsetof(user, u_debugreg[DR_CONTROL] );
    dr7_t dr7{};
    dr7.fields.dr0_len = (DR_LEN_4 >> 2);
    dr7.fields.dr0_break= DR_RW_READ;
    dr7.fields.dr0_local = 1;
    dr7.fields.reserved_10=1;
    check(ptrace(PTRACE_POKEUSER, tracee_id, dr0_offset, &value));
    check(ptrace(PTRACE_POKEUSER, tracee_id, dr7_offset, dr7.value));
}

void tracer(int tracee_id) {
    sleep(1);
    setup_breakpoint(tracee_id);
    puts("Tracer >> Hardware breakpoint set");
    check(ptrace(PTRACE_CONT, tracee_id, 0, 0));

    while (1) {
        int stat;
        check(waitpid(tracee_id, &stat, WUNTRACED));
        if (WIFSTOPPED(stat)) {
            int sig = WSTOPSIG(stat);
            if (sig == SIGTRAP) {
                auto dr6_offset = offsetof(user, u_debugreg[DR_STATUS]);
                auto dr6 = check(ptrace(PTRACE_PEEKUSER, tracee_id, dr6_offset, 0));
                if (dr6 & 1) {
                    puts("Tracer>> Hardware breakpoint hit");
                    sleep(1);
                    sig = 0; // do not send SIGTRAP
                }
            }
            check(ptrace(PTRACE_CONT, tracee_id, 0, sig));
        } else if (!WIFCONTINUED(stat))
            break;
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
