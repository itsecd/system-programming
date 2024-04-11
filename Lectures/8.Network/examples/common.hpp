//
// Created by alexey on 02.04.24.
//

#ifndef LECTORE8_COMMON_HPP
#define LECTORE8_COMMON_HPP

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <algorithm>
#include "check.hpp"

constexpr unsigned short SERVER_PORT = 60002;

inline std::ostream& operator<<(std::ostream& s, const sockaddr_in& addr) {
    union {
        in_addr_t x;
        char c[sizeof(in_addr)];
    }t{};
    t.x = addr.sin_addr.s_addr;
    return s << std::to_string(int(t.c[0]))
             << "." << std::to_string(int(t.c[1]))
             << "." << std::to_string(int(t.c[2]))
             << "." << std::to_string(int(t.c[3]))
             << ":" << std::to_string(ntohs(addr.sin_port));
}
inline int make_socket(int type) {
    switch (type)
    {
        case SOCK_STREAM:
            return socket(AF_INET, SOCK_STREAM, 0);
        case SOCK_DGRAM:
            return socket(AF_INET, SOCK_DGRAM, 0);
        case SOCK_SEQPACKET:
            return socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP); // analogue to SOCK_SEQPACKET
        default:
            errno = EINVAL;
            return -1;
    }
}
inline sockaddr_in local_addr(unsigned short port) {
    sockaddr_in addr{};
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);//127.0.0.1
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    return addr;
}

#pragma pack(push, 1)
struct Message{
    uint32_t x;
    char str[256];

    void to_host_order(){
        x = ntohl(x);
    }

    void to_net_order(){
        x = htonl(x);
    }

};
#pragma pack(pop)

inline std::ostream&  operator << (std::ostream& s, const Message& m){
    uint32_t len = std::find(m.str, m.str+sizeof(m.str), 0) - m.str;
    std::string_view  sv{&m.str[0], len};
    return s << "Message { x : " <<m.x << ",  str: '" << sv << "'}"<<std::endl;
}

inline void read_message( Message& m){


    while(true){
        std::cout << "Value of X: "<<std::flush;
        std::cin >> m.x;
        if(std::cin) break;
        if(std::cin.bad() || std::cin.eof()) exit(-2);
        std::cin.clear();
        std::cin.ignore(1024, '\n');
    }


    std::string str;
    std::cout << "Value of MSG:"<<std::flush;
    std::cin >> str;
    size_t size = std::min(str.size(), sizeof m.str -1);
    std::copy(str.c_str(), str.c_str()+size, &m.str[0]);
    m.str[size] = 0;
}


inline bool ask_continue(){
    std::cout << "Continue? " << std::endl;
    std::string str;
    std::cin >> str;
    return ( tolower(str[0]) == 'y');
}

#endif //LECTORE8_COMMON_HPP
