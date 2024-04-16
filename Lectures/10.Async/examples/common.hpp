#ifndef LECTORE8_COMMON_HPP
#define LECTORE8_COMMON_HPP

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <algorithm>
#include "check.hpp"
#include <chrono>

struct ScopedTimer{
    using clock_t = std::chrono::high_resolution_clock;
    using duration_t = std::chrono::duration<double, std::milli>;
    const std::string name;
    const clock_t::time_point start_time;
    explicit ScopedTimer(std::string_view  name);
    ~ScopedTimer();

};

constexpr unsigned short SERVER_PORT = 60002;

std::ostream& operator<<(std::ostream& s, const sockaddr_in& addr);

int make_socket(int type);

sockaddr_in local_addr(unsigned short port);

#pragma pack(push, 1)
struct Message{
    uint32_t x;
    char str[256];

    void to_host_order();

    void to_net_order();

};
#pragma pack(pop)

std::ostream&  operator << (std::ostream& s, const Message& m);

void read_message( Message& m);

bool ask_continue();

#endif //LECTORE8_COMMON_HPP
