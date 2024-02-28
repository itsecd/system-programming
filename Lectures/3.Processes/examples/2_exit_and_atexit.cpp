#include <unistd.h>
#include <stdio.h>
#include <cstdlib>

void on_end1() {
    puts("on_end1 >> But I'm not dead yet");
}
void on_end2() {
    puts("on_end2 >> Oh no, I'm dying...");
}
int main(int argc, char** argv) {
    if (atexit(on_end1)) {
        perror("Failed to register function on_end1");
        exit(EXIT_FAILURE);
    }
    if (atexit(on_end2)) {
        perror("Failed to register function on_end2");
        exit(EXIT_FAILURE);
    }

    int choice = 0;
    puts("0 - nothing, 1 - exit(), >1 - _exit(), <0 - abort(). Your choice?");
    scanf("%d", &choice);

    if(choice==1)
        exit(EXIT_SUCCESS);
    if(choice > 1)
        _exit(EXIT_SUCCESS);
    if(choice < 0)
        abort();
}