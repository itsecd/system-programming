#include "check.hpp"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fenv.h>
#include <sys/mman.h>

void fix_rip(ucontext_t* ctx){
    ctx->uc_mcontext.gregs[REG_RIP] += 16;
}

void sigfpe_handler(int signo, siginfo_t* info, void* ctx){

    if(info->si_code == SI_USER || info->si_code == SI_QUEUE){
        const char msg[] = ".........Caught SIGFPE sent by PID %d\n";
        printf(msg, info->si_pid);
        return;
    }

    auto address = info->si_addr;
    bool is_int = info->si_code == FPE_INTDIV;
    const char* reason = is_int? "Int 0/0" : "Float 0/0";

    // Non-reentrant - do not use if not sure!
    const char msg[] = ".........Caught SIGFPE at address %p (%s)\n";
    printf(msg, address, reason);

    fix_rip((ucontext_t*)ctx);
}


void sigsegv_handler(int signo, siginfo_t* info, void* ctx){
    auto fault_address = info->si_addr;
    auto instruction_address = reinterpret_cast<void*>(((ucontext_t*)ctx)->uc_mcontext.gregs[REG_RIP]);

    const char* reason = (info->si_code == SEGV_MAPERR) ? "(address not mapped)" : "(access violation)";

    const char msg[] = ".........Caught SIGSEGV at address %p when accessing %p %s\n";
    printf(msg, instruction_address, fault_address, reason);
    fix_rip((ucontext_t*)ctx);
}

#define NOPS asm volatile ("nop\n\t"\
"nop\n\t"\
"nop\n\t"\
"nop\n\t"\
"nop\n\t"\
"nop\n\t"\
"nop\n\t"\
"nop\n\t":::)


void zero_division(){
    volatile int x = 1;
    volatile int y = 0;
    x /= y; //[2]

    NOPS; NOPS; // signal handler returns here

    feenableexcept(FE_DIVBYZERO);
    volatile float z = 1;
    volatile float t = 0;
    z /= t;     //[3]

    NOPS; NOPS; // signal handler returns here
}

void null_dereference(){
    volatile char* volatile x= nullptr;
    *x = 1;   //[4]
    NOPS;NOPS;// signal handler returns here
}


void access_violation(){
    volatile char* volatile x = (char*) check(mmap(NULL, 4096, PROT_READ, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0));
    *x = 0;  // [4]
    NOPS;NOPS;
    munmap((void *) x, 4096);
}


int main(int argc, char**){
    struct sigaction s{};
    s.sa_sigaction = sigfpe_handler;
    s.sa_flags = SA_SIGINFO;
    check(sigaction(SIGFPE, &s, nullptr));

    s.sa_sigaction = sigsegv_handler;
    s.sa_flags = SA_SIGINFO;
    check(sigaction(SIGSEGV, &s, nullptr));

    kill(getpid(), SIGFPE);  // [1]

    zero_division();

    null_dereference();

    access_violation();
}