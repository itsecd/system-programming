#ifndef LECTURE10_UNIX_COMMON_HPP
#define LECTURE10_UNIX_COMMON_HPP
#include <sys/socket.h>
#include <sys/un.h>
#include "check.hpp"
#include <optional>

constexpr char UNIX_SOCKET_NAME[] = "/tmp/filesocket_";

inline int enable_passcred(int sockfd){
    int t = 1;
    return setsockopt(sockfd, SOL_SOCKET, SO_PASSCRED, &t, sizeof t); //enable credentials passing
}


template<int T>
struct ancillary_data{
    using type = void;
};

template<>
struct  ancillary_data<SCM_RIGHTS> {
    using type=int;
};

template<>
struct ancillary_data<SCM_CREDENTIALS> {
using type=ucred;
};

template< int TYPE>
std::optional<typename ancillary_data<TYPE>::type> get_ancillary_info(int sockfd){
    using RESULT_TYPE = ancillary_data<TYPE>::type;
    static_assert(TYPE == SCM_RIGHTS || TYPE == SCM_CREDENTIALS);
    char data_buffer[1];                                           //buffer
    iovec data_iov{.iov_base = data_buffer, .iov_len = 1};         //iovector pointing to a buffer
    alignas(cmsghdr) char spec_buffer[CMSG_SPACE(sizeof(RESULT_TYPE))];      //buffer for ancillary data
    std::fill_n(spec_buffer, sizeof spec_buffer, 0);
    msghdr msg{
            .msg_iov = &data_iov,.msg_iovlen = 1,                               //iovector(s) for ordinary data
            .msg_control = spec_buffer,.msg_controllen = sizeof(spec_buffer)};  //buffer for ancillary data

    check(recvmsg(sockfd, &msg, 0)); //receive the data
    auto cmsg = CMSG_FIRSTHDR(&msg);
    while(cmsg){
        if(cmsg->cmsg_type == TYPE)
            break;
        cmsg = CMSG_NXTHDR(&msg, cmsg);
    }
    if(!cmsg) //no ancillary info
        return std::nullopt;
    return *(RESULT_TYPE*)CMSG_DATA(cmsg);
}

#endif //LECTURE10_UNIX_COMMON_HPP
