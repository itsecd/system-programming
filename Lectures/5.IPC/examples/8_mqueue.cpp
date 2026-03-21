#include <cassert>

#include "common.hpp"
#include <mqueue.h>
#include <sys/wait.h>

const size_t MESSAGE_SIZE = sizeof(Message);

// function to calculate an absolute timeout time
timespec next_timeout(){
    constexpr timespec WAIT_TIME = {.tv_sec =0, .tv_nsec=100000};

    timespec result{};
    check(clock_gettime(CLOCK_REALTIME, &result)); // current time
    result.tv_sec += WAIT_TIME.tv_sec;
    result.tv_nsec += WAIT_TIME.tv_nsec;
    if(result.tv_nsec > 1'000'000'000)
    {
        result.tv_sec += result.tv_nsec / 1'000'000'000;
        result.tv_nsec %= 1'000'000'000;
    }
    return result;
}

void reader(mqd_t fd, pid_t writer_pid){
    while(true) {
        Message m{};

        ssize_t msg_size = -1;
        do {
            auto timeout_time = next_timeout();
            msg_size = mq_timedreceive(fd, (char*)&m, sizeof m, nullptr, &timeout_time);

            if (msg_size < 0)
                check_except(-1, ETIMEDOUT); // kill process on any error except ETIMEDOUT

            if (not is_parent_alive(writer_pid))
                exit(EXIT_SUCCESS);
        } while (msg_size < 0);

        assert(msg_size == MESSAGE_SIZE); // partial reads do not occur on queues -> error

        COUT << m << std::endl;
    }
}


void writer(mqd_t fd, pid_t reader_pid){
    timespec t{.tv_sec =0, .tv_nsec=100000};
    while(true){
        Message m{};
        read_message(m);

        ssize_t result = -1;

        do {
            auto timeout_time = next_timeout();
            result = mq_timedsend(fd, (char*)&m, sizeof m, 0, &timeout_time);
            check_except(result, ETIMEDOUT); // if failed and errno != ETIMEDOUT -> abort
            if (not is_child_alive(reader_pid))
                exit(EXIT_FAILURE);
        }while (result < 0);

        nanosleep(&t, nullptr);
        if (!ask_continue())
            exit(EXIT_SUCCESS);
    }
}

const char MQ_NAME[] = "/MQUEUE_";

int main(){
    const mq_attr attr{.mq_maxmsg = 1, .mq_msgsize = MESSAGE_SIZE};
    mq_unlink(MQ_NAME); // delete old queue if it exists

    mqd_t  fd = check(mq_open(MQ_NAME, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR, &attr ));
    pid_t writer_pid = getpid();
    pid_t reader_pid = check(fork());

    if(reader_pid)
        writer(fd, reader_pid);
    else
        reader(fd, writer_pid);
}