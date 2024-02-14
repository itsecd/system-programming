#include <sys/inotify.h>
#include <unistd.h>
#include "check.hpp"
#include <stdlib.h>

void handle_event(inotify_event *event) {

    if(event->len)
        printf("%s", event->name);
    else
        fputs("Watched directory", stdout);

    if(event->mask & IN_CREATE){
        puts(" created");
    }
    if(event->mask & IN_DELETE){
        puts(" deleted ");
    }
    if(event->mask & IN_MOVED_TO){
        puts(" moved in ");
    }
    if(event->mask & IN_MOVED_FROM){
        puts(" moved out ");
    }
    if(event->mask & IN_DELETE_SELF){
        puts(" deleted");
    }
    if(event->mask & IN_MOVE_SELF){
        puts(" moved");
    }
    if(event->mask & IN_ACCESS){
        puts(" accessed");
    }
    if(event->mask & IN_MODIFY){
        puts(" modified");
    }
    if(event->mask & IN_OPEN){
        puts(" opened");
    }
    if(event->mask & IN_CLOSE)
    {
        puts(" closed");
    }

}

void handle_events(char* buffer, ssize_t size){
    auto end = buffer + size;
    char* cur = buffer;
    while(cur < end){
        inotify_event* event = (inotify_event*)cur;
        handle_event(event);
        cur += sizeof(inotify_event)+event->len;
    }

}

char buf[sizeof(inotify_event) + 4096 + 1];

int main(int argc, char** argv){

    char* dirname;
    if(argc > 1){
        dirname = argv[1];
    }
    else{
        char message[] = "Not enough arguments";
        write(STDERR_FILENO, message, sizeof message);
        exit(0);
    }

    int fd = check(inotify_init());
    int wd = check(inotify_add_watch(fd, dirname,
                               IN_ONLYDIR|IN_CREATE|IN_DELETE|IN_DELETE_SELF|IN_CLOSE|IN_MOVE|IN_MOVE_SELF|IN_OPEN|IN_ACCESS|IN_MODIFY ));

    while(true){
        auto read_size = check(read(fd, buf, sizeof (buf)));

        handle_events(buf, read_size);
    }

}