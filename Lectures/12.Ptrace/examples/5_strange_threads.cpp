#include "check.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <iostream>
#include <fcntl.h>
#include <set>
#include <sys/mman.h>
#include <sys/wait.h>

std::set<int> opened_descriptors;
volatile int x;

std::string get_cwd(){
    char buf[4096]{};
    getcwd(buf,sizeof buf);

    return buf;
}

void try_read_files(){
    char c;
    for (auto fd: opened_descriptors)
        if(read(fd,&c, 1) >= 0)
            std::cout << "FD " << fd << " READ SUCCESSFUL; NEW RW POSITION "<< lseek(fd,  0,SEEK_CUR) << std::endl;
        else
            std::cout << "FD " << fd << " READ FAILED\n";
}

void* thread_fn(void* arg){
    //sleep(1);
    ++x;
    std::cout << "CHILD: PID " << getpid() << " TID " << gettid() << " PthreadTID " << pthread_self() << std::endl;

    int fd = check(open("/etc/passwd", O_RDONLY));
    std::cout << "OPENED FD " << fd << std::endl;
    opened_descriptors.insert(fd);

    try_read_files();

    auto cwd = get_cwd();
    std::cout << "OLD CWD " << cwd << std::endl;
    auto new_cwd = cwd + "/..";
    chdir(new_cwd.c_str());

    std::cout << "NEW CWD " << get_cwd()<< std::endl;

//    chdir(cwd.c_str());
//    std::cout << "CWD REVERTED" << std::endl;
    return nullptr;
}

int thread_fn2(void*){
    thread_fn(nullptr);
    return 0;
}

void spawn_posix_thread(){
    pthread_t tid;
    check_result(pthread_create(&tid, nullptr, thread_fn, nullptr));
    check_result(pthread_join(tid, nullptr));
}

auto alloc_stack(){
    auto ptr = mmap(nullptr, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_GROWSDOWN|MAP_STACK, -1, 0);
    if(ptr == MAP_FAILED)
        check(-1);
    ptr = (char*)ptr + 8192; // move ptr to the top address
    return ptr;
}

void spawn_regular_thread(){

    auto stack = alloc_stack();
    check(clone(thread_fn2, stack, CLONE_THREAD|CLONE_SIGHAND|CLONE_VM|CLONE_FILES|CLONE_FS, nullptr));
    sleep(2); // can not wait() or join()
}

void spawn_strange_thread(){
    auto stack = alloc_stack();
    check(clone(thread_fn2, stack, CLONE_THREAD|CLONE_SIGHAND|CLONE_VM, nullptr));
    sleep(2); // can not wait() or join()
}

void spawn_strange_process(){
    auto stack = alloc_stack();
    //                                                 ! signal to send when the process ends
    int pid = check(clone(thread_fn2, stack, CLONE_VM|SIGCHLD, nullptr));
    check(wait(nullptr));
}

int main(){
    void(*functions[])()  {spawn_posix_thread, spawn_regular_thread, spawn_strange_thread, spawn_strange_process};
    for (auto f: functions){
        std::cout << "=======" << std::endl;
        f();
        std::cout << "=======\nPARENT\n";
        std::cout << "CWD " << get_cwd() << std::endl;
        try_read_files();
        std::cout << "VALUE OF X: " << x<< std::endl;
    }
}
