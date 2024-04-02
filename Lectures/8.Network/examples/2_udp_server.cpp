#include "common.hpp"



int main() {
    Message msg{};

    auto server_address = local_addr(SERVER_PORT);

    auto socket = check(make_socket(SOCK_DGRAM));
    check(bind(socket, (sockaddr*)&server_address, sizeof(server_address)));

    while (true) {
        sockaddr_in sender_address{};
        socklen_t addrlen = sizeof(sender_address);

        while (true) {
            auto size = check(recvfrom(socket, &msg, sizeof(msg), 0,
                                       (sockaddr*)&sender_address, &addrlen));
            if (size < 1)
                continue; //disconnected
            msg.to_host_order();
            std::cout
                    << sender_address
                    << " sent a message " << msg << std::endl;
        }
    }
}