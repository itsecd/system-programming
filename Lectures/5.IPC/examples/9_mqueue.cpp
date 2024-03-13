#include "common.hpp"
#include <mqueue.h>
#include <sys/wait.h>

const size_t MESSAGE_SIZE = sizeof(Message);

void reader(mqd_t fd, pid_t parent_pid){
    timespec t{.tv_sec =0, .tv_nsec=100000};
    while(true) {
        Message m{};

        while(check_except(
                mq_timedreceive(fd, (char*)&m, sizeof m, NULL, &t)
                , ETIMEDOUT) < 0){
            if(parent_pid != getppid())
                exit(0);
        }

        COUT << m << std::endl;
    }
}


void writer(mqd_t fd){
    timespec t{.tv_sec =0, .tv_nsec=100000};
    while(true){
        Message m{};
        read_message(m);

        while(check_except(
                      mq_timedsend(fd, (char*)&m, sizeof m, NULL, &t)
              , ETIMEDOUT) < 0){
            if(waitpid(0, NULL, WNOHANG) != 0)
                exit(0);
        }

        nanosleep(&t, nullptr);
        ask_continue();
    }
}

const char MQ_NAME[] = "/MQUEUE_";

int main(){
    mq_attr attr{.mq_maxmsg = 1, .mq_msgsize = MESSAGE_SIZE};
    mq_unlink(MQ_NAME);

    mqd_t  fd = check(mq_open(MQ_NAME, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR, &attr ));
    pid_t writer_pid = getpid();
    pid_t reader_pid = check(fork());

    if(reader_pid)
        writer(fd);
    else
        reader(fd, writer_pid);
}