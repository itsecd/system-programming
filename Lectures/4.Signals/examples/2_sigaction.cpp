#include "check.hpp"
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t last_sig;

void sig_handler(int signo) {
    //better not call printf here because it’s not reentrant
    last_sig = signo;
}

void child() {

    printf("Сын >> Ыыыыыыыыыыы >:>\n");
    struct sigaction abrt_action {}
    , quit_action{};

    //ignore SIGABRT
    abrt_action.sa_handler = SIG_IGN;
    check(sigaction(SIGABRT, &abrt_action, NULL));
    puts("Сын >> Игнорирую SIGABRT");

    //handle SIGQUIT
    quit_action.sa_handler = sig_handler;
    check(sigaction(SIGQUIT, &quit_action, NULL));
    puts("Сын >> Жду SIGQUIT");

    sigset_t  set;
    sigemptyset(&set);

    while (true) {
        sigsuspend(&set); //wait for any non-blocked signal
        printf("Сын >> Сиигнаааал %d \n", last_sig);
    }

}

bool process_exists(pid_t p) {
    if(kill(p, 0) == -1 && errno == ESRCH)
        return false;
    return true;
}


void try_kill(pid_t p, int sig) {
    if (sig < SIGRTMIN)
        check(kill(p, sig));
    else
        check(sigqueue(p, sig, sigval{ 1024 }));
    sleep(1);
    if (!process_exists(p)) {
        printf("Отец >> Убил наконец...\n");
        exit(EXIT_SUCCESS);
    }
    printf("Отец >> Выжил, однако...\n");
}

void no_zombie(){
    struct sigaction s{};
    s.sa_handler = SIG_IGN;
    check(sigaction(SIGCHLD, &s, NULL));
}

void parent(pid_t child) {
    no_zombie();

    printf("Отец >> Йа тебя породил, йа тебя и убью!\n");
    sleep(1);
    printf("Отец >> Попробуем SIGABRT\n");
    try_kill(child, SIGABRT);
    printf("Отец >> Попробуем SIGQUIT\n");
    try_kill(child, SIGQUIT);

    printf("Отец >> SIGKILL точно убьет\n");
    try_kill(child, SIGKILL);
}


int main() {
    pid_t child_id = check(fork());
    if (child_id)
        parent(child_id);
    else
        child();
}