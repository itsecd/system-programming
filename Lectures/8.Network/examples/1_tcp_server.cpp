#include "common.hpp"



int main() {
    Message msg{};

    auto server_address = local_addr(SERVER_PORT);

    auto listening_socket = check(make_socket(SOCK_STREAM));
    check(bind(listening_socket, (sockaddr*)&server_address, sizeof(server_address)));
    check(listen(listening_socket, 2));

    int connected_socket = 0;

    while (true) {
        sockaddr_in connected_address{};
        socklen_t addrlen = sizeof(connected_address);
        connected_socket = check(accept(listening_socket, (sockaddr*)&connected_address,
                                        &addrlen));
        std::cout << "Connected from " << connected_address << std::endl;
        while (true) {
            auto size = check_except(recv(connected_socket, &msg, sizeof(msg), MSG_WAITALL), ENOTCONN);
            if (size < 1)
                break; //disconnected
            msg.to_host_order();
            std::cout
                    << connected_address
                    << " send a message " << msg << std::endl;
        }
        std::cout << "Disconnected from " << connected_address << std::endl;
        close(connected_socket);
    }
}