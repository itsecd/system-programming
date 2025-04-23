#include <unistd.h>
#include "unix_common.hpp"
#include <fcntl.h>
#include <iostream>

const uid_t ALLOWED_UID = getuid();


void send_file(int sockfd, int filefd){
    char data_buffer[1];                                           //buffer
    iovec data_iov{.iov_base = data_buffer, .iov_len = 1};         //iovector pointing to a buffer
    alignas(cmsghdr) char spec_buffer[CMSG_SPACE(sizeof(int))] {};    //buffer for ancillary data

    msghdr msg{
            .msg_iov = &data_iov,.msg_iovlen = 1,                               //iovector(s) for ordinary data
            .msg_control = spec_buffer,.msg_controllen = sizeof(spec_buffer)};  //buffer for ancillary data

    cmsghdr* chdr = CMSG_FIRSTHDR(&msg);
    chdr->cmsg_len = CMSG_LEN(sizeof(int));
    chdr->cmsg_level = SOL_SOCKET; //UNIX socket level
    chdr->cmsg_type = SCM_RIGHTS; //transferring descriptors
    *(int*)CMSG_DATA(chdr) = filefd;
    check(sendmsg(sockfd, &msg, 0));
}

bool authenticate(int sockfd){
    check(send(sockfd, &sockfd, 1, 0)); //send 1 byte to indicate that we're ready to receive creds
    auto cred = get_ancillary_info< SCM_CREDENTIALS>(sockfd); //get auth info

    if(cred.has_value()){
        std::cout << "Got request from UID " << cred->uid<< std::endl;
        std::cout << "Access " << (cred->uid == ALLOWED_UID ? "granted" : "rejected") <<std::endl;
        return cred->uid == ALLOWED_UID;
    }
    else{
        std::cout << " No auth data were passed " << std::endl;
        return false;
    }
}

int make_hidden_file(){
    char name[] = "/tmp/us_XXXXXX";
    int fd = check(mkstemp(name));
    int rdonly_fd = check(open(name, O_RDONLY));
    unlink(name);
    char data[] = "6311_6312_6313";
    check(write(fd, data, sizeof data -1));
    close(fd);

    return rdonly_fd;
}

int make_socket() {
    int listen_fd = check(socket(AF_UNIX, SOCK_STREAM, 0)); //creating socket;
    unlink(UNIX_SOCKET_NAME);
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, UNIX_SOCKET_NAME, sizeof UNIX_SOCKET_NAME);
    check(bind(listen_fd, (sockaddr *) &addr, sizeof addr));
    check(enable_passcred(listen_fd));
    check(listen(listen_fd, 10));
    return listen_fd;
}


int main(){
    const auto rdonly_fd = make_hidden_file();
    int listen_fd = make_socket();
    while(true){
        int sockfd = check(accept(listen_fd, nullptr, nullptr));
        if(authenticate(sockfd))
        {
            send_file(sockfd,rdonly_fd);
        }
        else{
            char c;
            check(send(sockfd,&c, 1, 0)); //send message without the file descriptor
        }
        close(sockfd);
    }
}
