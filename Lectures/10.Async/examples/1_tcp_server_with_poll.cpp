#include <vector>
#include <unordered_map>
#include "common.hpp"
#include <poll.h>

[[noreturn]]
int main() {

    std::unordered_map<int, sockaddr_in> clients;

    auto server_address = local_addr(SERVER_PORT);

    auto listening_socket = check(make_socket(SOCK_STREAM));
    check(bind(listening_socket, (sockaddr *) &server_address, sizeof(server_address)));
    check(listen(listening_socket, 2));


    std::vector<pollfd> watched_fds{  pollfd{listening_socket,  POLLIN}  };
    while (true) {
        while (check(poll(watched_fds.data(), watched_fds.size(), -1)) == 0) {}

        if(watched_fds[0].revents & POLLIN){
            int connected_socket = 0;
            sockaddr_in connected_address{};
            socklen_t addrlen = sizeof(connected_address);
            connected_socket = check(accept(listening_socket, (sockaddr *) &connected_address,
                                            &addrlen));
            watched_fds.push_back({connected_socket, POLLIN });
            clients.insert({connected_socket, connected_address});
            std::cout << connected_address << " connected" << std::endl;
        }

        for (auto &wfd: watched_fds) {
            if(wfd.fd == listening_socket){
                continue;
            }
            const auto &address = clients[wfd.fd];

            if (wfd.revents & POLLIN) {
                bool read_any = false;
                while(true) {
                    Message m{};
                    auto size = check_except(recv(wfd.fd, &m, sizeof m, MSG_PEEK | MSG_DONTWAIT), EAGAIN);
                    if(size == 0 && !read_any) {
                        wfd.revents |= POLLHUP; // we were awoken, but there is no data - disconnected
                    }
                    if(size != sizeof m) break; // partial or empty data
                    read_any = true;
                    check(recv(wfd.fd, &m, sizeof m, 0));
                    m.to_host_order();
                    std::cout
                            << address
                            << " sent a message " << m << std::endl;
                }
            }

            if (wfd.revents & (POLLHUP|POLLERR)) {
                close(wfd.fd);

                std::cout << "Disconnected from " << address << std::endl;
                clients.erase(wfd.fd);
                wfd.fd = -1;
            }
        }

        watched_fds.erase(std::remove_if(watched_fds.begin(), watched_fds.end(),
                                         [](const auto &wfd) { return wfd.fd == -1; }),
                          watched_fds.end());

    }
}