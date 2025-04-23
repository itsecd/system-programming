#include "unistd.h"
#include <cstdio>
#include "unix_common.hpp"


void authenticate(int sockfd){
        int t;
        check(recv(sockfd, &t, 1, 0));//wait the server
        check(send(sockfd, &t, 1, 0));//send 1 byte (credentials are passed automatically)
}

int get_fd(){
    int sockfd = check(socket(AF_UNIX, SOCK_STREAM, 0)); //creating socket;
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, UNIX_SOCKET_NAME, sizeof UNIX_SOCKET_NAME);
    check(connect(sockfd, (sockaddr*)&addr, sizeof addr)); //connecting to an existing socket
    authenticate(sockfd);
    auto fd = get_ancillary_info< SCM_RIGHTS>(sockfd);
    return fd ? *fd : -1;
}

int main(){
    int fd = get_fd();
    if(fd < 0)
    {
        puts("Failed to get the file descriptor");
        exit(EXIT_FAILURE);
    }
    char buffer[1024];
    ssize_t data_size = 0;
    do{
        data_size =check(read(fd, &buffer, sizeof buffer));
        check(write(STDOUT_FILENO, &buffer, data_size ));

    }while(data_size > 0);
    check(lseek(fd, SEEK_SET, 0));
}

