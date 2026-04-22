#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "check.hpp"
#include "common.hpp"
#include "memory"

constexpr size_t CHUNK_SIZE = 64*1024;
constexpr size_t CHUNKS_COUNT = 16*1024;

const sockaddr_in NET_SERVER_ADDR = {
    .sin_family = AF_INET,
    .sin_port = htons(64000),
    .sin_addr = in_addr{ .s_addr = inet_addr("127.0.0.1")
    }
};

constexpr sockaddr_un UNIX_SERVER_ADDR = {
    .sun_family = AF_UNIX,
    .sun_path = "/tmp/test_socket"
};

using duration_t = std::chrono::duration<double>;
using hr_clock = std::chrono::high_resolution_clock;

int make_socket(int domain)
{
    int sock_fd = check(socket(domain, SOCK_STREAM, 0));
    return sock_fd;
}

void run_server(int domain)
{
    std::unique_ptr<char[]> buffer { new char[CHUNK_SIZE] };

    int sockfd = make_socket(domain);
    const sockaddr* addr;
    socklen_t addr_len;
    if (domain == AF_UNIX)
    {
        unlink(UNIX_SERVER_ADDR.sun_path);
        addr = (const sockaddr*)&UNIX_SERVER_ADDR;
        addr_len = sizeof UNIX_SERVER_ADDR;
    }
    else
    {
        addr = (const sockaddr*)&NET_SERVER_ADDR;
        addr_len = sizeof NET_SERVER_ADDR;
    }
    check(bind(sockfd, addr, addr_len));
    check(listen(sockfd, 1));

    int fd = check(accept(sockfd, nullptr, nullptr));

    size_t read_size = 0;
    duration_t duration{};
    {
        auto start = hr_clock::now();

        while (true)
        {
            size_t r = read(fd, buffer.get(), CHUNK_SIZE);
            read_size += r;
            if (r == 0) break;
        }
        auto end = hr_clock::now();
        duration =  end - start;
    }
    auto throughput = read_size * 8.0 / (1000*1000*1000) / duration.count();
    std::cout << "Throughput " << throughput << " Gbit/s"<<std::endl;

    close(fd);
    close(sockfd);
}

void run_client(int domain)
{
    std::unique_ptr<char[]> buffer { new char[CHUNK_SIZE] };
    for (int i = 0; i < CHUNK_SIZE; ++i)
        buffer[i] = i;
    int sockfd = make_socket(domain);
    const sockaddr* addr;
    socklen_t addr_len;
    if (domain == AF_UNIX)
    {
        addr = (const sockaddr*)&UNIX_SERVER_ADDR;
        addr_len = sizeof UNIX_SERVER_ADDR;
    }
    else
    {
        addr = (const sockaddr*)&NET_SERVER_ADDR;
        addr_len = sizeof NET_SERVER_ADDR;
    }
    sleep(1);
    check(connect(sockfd, addr, addr_len));

    for (size_t i = 0; i < CHUNKS_COUNT; ++i)
        check(write(sockfd, buffer.get(), CHUNK_SIZE));

    shutdown(sockfd,SHUT_WR);
    close(sockfd);
}

void run(int domain)
{
    if (check(fork()))
    {
        run_client(domain);
        check(wait(nullptr));
    }
    else
    {
        run_server(domain);
        exit(0);
    }

}

int main()
{
    std::cout << "UNIX"<<std::endl;
    run(AF_UNIX);
    std::cout << "TCP"<<std::endl;
    run(AF_INET);
}