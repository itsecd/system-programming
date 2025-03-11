#include "common.hpp"
#include <mqueue.h>
#include <sys/wait.h>

const size_t MESSAGE_SIZE = sizeof(Message);

// function to calculate an absolute timeout time
timespec next_timeout(timespec wait_time){
    timespec result{};
    check(clock_gettime(CLOCK_REALTIME, &result)); // current time
    result.tv_sec += wait_time.tv_sec;
    result.tv_nsec += wait_time.tv_nsec;
    if(result.tv_nsec > 1'000'000'000)
    {
        result.tv_sec += result.tv_nsec / 1'000'000'000;
        result.tv_nsec %= 1'000'000'000;
    }
    return result;
}

void reader(mqd_t fd, pid_t parent_pid){
    timespec t{.tv_sec =0, .tv_nsec=100000};
    while(true) {
        Message m{};

        auto timeout_time = next_timeout(t);
        ssize_t r;
        while(r = mq_timedreceive(fd, (char*)&m, sizeof m, nullptr, &timeout_time), r < 0){
            check_except(r, ETIMEDOUT); // if errno != ETIMEDOUT -> abort
            if(parent_pid != getppid())
                exit(0);
            timeout_time = next_timeout(t);
        }

        COUT << m << std::endl;
    }
}


void writer(mqd_t fd){
    timespec t{.tv_sec =0, .tv_nsec=100000};
    while(true){
        Message m{};
        read_message(m);

        auto timeout_time = next_timeout(t);


        int r;
        while(r = mq_timedsend(fd, (char*)&m, sizeof m, 0, &timeout_time), r != 0){
            check_except(r, ETIMEDOUT); // if errno != ETIMEDOUT -> abort
            if(waitpid(0, nullptr, WNOHANG) != 0)
                exit(-1);
            timeout_time = next_timeout(t);
        }

        // message can be placed to queue even if the writer is dead -> check after sending
        if(waitpid(0, nullptr, WNOHANG) != 0)
            exit(-1);

        nanosleep(&t, nullptr);
        ask_continue();
    }
}

const char MQ_NAME[] = "/MQUEUE_";

int main(){
    mq_attr attr{.mq_maxmsg = 1, .mq_msgsize = MESSAGE_SIZE};
    mq_unlink(MQ_NAME); // delete old queue if it exists

    mqd_t  fd = check(mq_open(MQ_NAME, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR, &attr ));
    pid_t writer_pid = getpid();
    pid_t reader_pid = check(fork());

    if(reader_pid)
        writer(fd);
    else
        reader(fd, writer_pid);
}