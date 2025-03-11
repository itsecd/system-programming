#include <unistd.h>
#include "check.hpp"
#include <memory.h>
#include <wait.h>
#include <limits.h>

const size_t BIG_MESSAGE_SIZE = PIPE_BUF + 1;
const size_t SMALL_MESSAGE_SIZE = PIPE_BUF - 1;

const size_t MESSAGES_COUNT = 10;
const size_t WRITERS_COUNT = 4;


void writer(int write_fd, size_t msg_size, char value){
    char* buffer = (char*)malloc(msg_size);
    memset(buffer, value, msg_size);

    for(size_t i = 0; i < MESSAGES_COUNT; ++i)
        check(write(write_fd, buffer, msg_size));

}

void reader(int read_fd, size_t msg_size){
    char* buffer = (char*)malloc(msg_size);

    for(size_t i = 0; i < MESSAGES_COUNT*WRITERS_COUNT; ++i) {

        size_t current_pos = 0;
        while(current_pos != msg_size)
            current_pos += check(read(read_fd, buffer+current_pos, msg_size-current_pos));

        bool ok = true;
        for(size_t j = 1; j < msg_size; ++j){
            if(buffer[0] != buffer[j]) {
                ok = false;
                break;
            }
        }

        puts(ok ? "Good message" : "Corrupted message");


    }

    free(buffer);

}

int main()
{
    int pipefd[2];
    check(pipe(pipefd));
    auto [read_fd, write_fd] = pipefd;

    printf("Small message size: %lu \nBig message size: %lu\n", SMALL_MESSAGE_SIZE, BIG_MESSAGE_SIZE);
    puts("\n============Small messages============:");

    for(int i = 0; i < WRITERS_COUNT; ++i){
        if(check(fork()) == 0) {
            writer(write_fd, SMALL_MESSAGE_SIZE, 'A' + i);
            exit(0);
        }
    }
    reader(read_fd, SMALL_MESSAGE_SIZE);
    while(check_except(wait(NULL), ECHILD) >= 0) {}    // WAIT for all children


    puts("\n============Big messages==============:");

    for(int i = 0; i < WRITERS_COUNT; ++i){
        if(check(fork()) == 0) {
            writer(write_fd, BIG_MESSAGE_SIZE, 'A' + i);
            exit(0);
        }
    }
    reader(read_fd, BIG_MESSAGE_SIZE);
    while(check_except(wait(NULL), ECHILD) >= 0) {}    // WAIT for all children

}

