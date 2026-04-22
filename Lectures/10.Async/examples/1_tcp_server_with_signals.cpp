#include <vector>
#include <unordered_map>
#include "common.hpp"
#include <signal.h>
#include <fcntl.h>

const int IO_SIGNAL = SIGRTMIN;

void setup_async_mode(int fd)
{
    check(fcntl(fd, F_SETOWN, getpid()));
    check(fcntl(fd, F_SETSIG, IO_SIGNAL));
    auto old_flags =  fcntl(fd, F_GETFL);
    check(fcntl(fd, F_SETFL, old_flags | O_ASYNC ));
}

[[noreturn]]
int main() {
    sigset_t wait_set;
    sigemptyset(&wait_set);
    sigaddset(&wait_set, IO_SIGNAL);
    check(sigprocmask(SIG_BLOCK,&wait_set,  nullptr));

    std::unordered_map<int, sockaddr_in> clients;

    auto server_address = local_addr(SERVER_PORT);

    auto listening_socket = check(make_socket(SOCK_STREAM));
    setup_async_mode(listening_socket);
    check(bind(listening_socket, (sockaddr *) &server_address, sizeof(server_address)));
    check(listen(listening_socket, 2));

    while (true) {

        siginfo_t info{};
        check(sigwaitinfo(&wait_set, &info));

        int current_fd = info.si_fd;

        if( current_fd== listening_socket){
            int connected_socket = 0;
            sockaddr_in connected_address{};
            socklen_t addrlen = sizeof(connected_address);
            connected_socket = check(accept(listening_socket, (sockaddr *) &connected_address,
                                            &addrlen));
            setup_async_mode(connected_socket);
            clients.insert({connected_socket, connected_address});
            std::cout << connected_address << " connected" << std::endl;
            continue;
        }

        bool disconnected = false;

        if (clients.contains(current_fd))
        {
            bool read_any = false;
            while(true)
            {
                auto& address = clients[current_fd];
                Message m{};
                auto size = check_except(recv(current_fd, &m, sizeof m, MSG_PEEK | MSG_DONTWAIT), EAGAIN);
                if(size == 0 && !read_any) {
                    disconnected = true;
                }
                if(size != sizeof m) break; // partial or empty data
                check(recv(current_fd, &m, sizeof m, 0));
                read_any = true;
                m.to_host_order();
                std::cout
                        << address
                        << " sent a message " << m << std::endl;
            }
        }

        if (disconnected)
        {
            close(current_fd);
            auto& address = clients[current_fd];
            std::cout << "Disconnected from " << address << std::endl;
            clients.erase(current_fd);
        }

    }
}