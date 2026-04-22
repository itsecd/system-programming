#ifndef LECTURE10_UNIX_COMMON_HPP
#define LECTURE10_UNIX_COMMON_HPP
#include <sys/socket.h>
#include <sys/un.h>
#include "check.hpp"
#include <optional>

constexpr char UNIX_SOCKET_NAME[] = "/tmp/filesocket_";


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
std::optional<typename ancillary_data<TYPE>::type> recv_ancillary_info(int sockfd){
    static_assert(TYPE == SCM_RIGHTS || TYPE == SCM_CREDENTIALS);
    using result_t = ancillary_data<TYPE>::type;

    char data_buffer[1];                                           //buffer
    iovec data_iov{.iov_base = data_buffer, .iov_len = 1};         //iovector pointing to a buffer
    alignas(cmsghdr) char spec_buffer[CMSG_SPACE(sizeof(result_t))];      //buffer for ancillary data
    std::fill_n(spec_buffer, sizeof spec_buffer, 0);
    msghdr msg{
            .msg_iov = &data_iov,//iovector(s) for ordinary data
            .msg_iovlen = 1,
            .msg_control = spec_buffer,//buffer for ancillary data
            .msg_controllen = sizeof(spec_buffer)};

    check(recvmsg(sockfd, &msg, 0)); //receive the data
    auto cmsg = CMSG_FIRSTHDR(&msg);
    while(cmsg){
        if(cmsg->cmsg_type == TYPE)
            break;
        cmsg = CMSG_NXTHDR(&msg, cmsg);
    }
    if(!cmsg) //no ancillary info
        return std::nullopt;
    return *(result_t*)CMSG_DATA(cmsg);
}

#endif //LECTURE10_UNIX_COMMON_HPP
