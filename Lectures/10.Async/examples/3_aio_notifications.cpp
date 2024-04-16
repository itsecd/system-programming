#include "common.hpp"
#include <aio.h>
#include <signal.h>
#include <fcntl.h>
#include <numeric>
#include <span>


struct ThreadArgs{
    aiocb* cb;
    char* buffer;
};

double mean(std::span<const char> data){
    return std::accumulate(data.begin(), data.end(), 0.0)/data.size();
}

void thread_fn(sigval_t value){
    std::cout << "Thread started " << std::endl;
    auto* args = (ThreadArgs*)value.sival_ptr;

    auto size = check(aio_return(*&args->cb));

    std::cout << mean({args->buffer, (size_t)size})<< std::endl;
    std::cout << "Thread completed " << std::endl;
    exit(0);
}

void handler(int sig, siginfo_t* info, void*){
    std::cout << "Handler called with value "<< info->si_value.sival_int<<std::endl;
}

int main(){

    {
        struct sigaction a{};
        a.sa_sigaction = handler;
        a.sa_flags = SA_SIGINFO;
        check(sigaction(SIGIO, &a, nullptr));

        sigset_t set{};
        sigaddset(&set, SIGIO);
        check(sigprocmask(SIG_BLOCK, &set, nullptr));
    }

    char buffer[1024];
    auto fd = check(open("/dev/random", O_RDONLY));

    aiocb cb0{
            .aio_fildes = fd,
            .aio_buf = buffer,
            .aio_nbytes=sizeof(buffer)
    };

    aiocb cb1{
            .aio_fildes = fd,
            .aio_buf = buffer,
            .aio_nbytes=sizeof(buffer),
            .aio_sigevent = {.sigev_value = {1234},
                             .sigev_signo = SIGIO,
                             .sigev_notify = SIGEV_SIGNAL,
                             },
    };

    aiocb cb2 {
            .aio_fildes = fd,
            .aio_buf = buffer,
            .aio_nbytes=sizeof(buffer),
            .aio_sigevent = {
                    .sigev_value = {.sival_ptr = new ThreadArgs{&cb2, buffer}},
                    .sigev_notify = SIGEV_THREAD,
            },
    };
    cb2.aio_sigevent.sigev_notify_function =thread_fn;


    std::cout << "Starting first request"<< std::endl;
    check(aio_read(&cb0));
    aiocb* wait_list[] = {&cb0};
    check(aio_suspend(wait_list, 1, nullptr));
    auto size = check(aio_return(&cb0));
    std::cout << mean({buffer, (size_t)size}) << std::endl;
    std::cout << "First request completed"<< std::endl;

    std::cout << "Starting second request"<< std::endl;
    check(aio_read(&cb1));
    {
        sigset_t set{};
        sigemptyset(&set);
        sigsuspend(&set);
    }
    size = check(aio_return(&cb1));
    std::cout << mean({buffer, (size_t)size}) << std::endl;
    std::cout << "Second request completed"<< std::endl;

    std::cout << "Starting third request"<< std::endl;
    check(aio_read(&cb2));

    sleep(-1);
}