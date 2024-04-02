#include "common.hpp"

int main() {
    auto dest_address = local_addr(SERVER_PORT);
    int sock_fd = check(make_socket(SOCK_DGRAM));
    check(connect(sock_fd, (sockaddr * ) & dest_address, sizeof(dest_address))); // only sets the default destination address
    Message msg{};
    while (1) {
        read_message(msg);
        msg.to_net_order();
        send(sock_fd, &msg, sizeof msg, MSG_WAITALL);
        if(!ask_continue())
            break;
    }

    close(sock_fd);
}
