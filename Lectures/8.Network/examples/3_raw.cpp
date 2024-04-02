#include "check.hpp"
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <iostream>
#include <arpa/inet.h>
#include <chrono>

in_addr_t TARGET_ADDR;
const unsigned short ID = 'IB';
const unsigned char MAX_HOPS = 128;
const auto WAIT_TIME = std::chrono::milliseconds(500);

unsigned short inet_checksum(const unsigned short* data, size_t count){
    unsigned long result = 0;
    for(size_t i = 0; i < count; ++i)
        result += ntohs(data[i]);
    result += (result >> 16);
    return htons(~(unsigned short)result);
}

static icmphdr create_echo_request( unsigned short id, unsigned short sequence_number){
        icmphdr result{}; //zero fill
        result.type = ICMP_ECHO;
        result.un.echo.id = htons(id);
        result.un.echo.sequence = htons(sequence_number);

        result.checksum = inet_checksum((unsigned short*)&result, sizeof(result)/sizeof(short));

        auto sum = inet_checksum((unsigned short*)&result, sizeof(result)/sizeof(short));
        if(sum != 0)
            std::cerr <<"Wrong ICMP-header checksum!"<<std::endl;

        return result;
    }


int make_icmp_socket(){
    auto sockfd = check(socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)); // get only ICMP messages

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    check(bind(sockfd, (sockaddr*)&addr, sizeof(addr)));

    return sockfd;
}

auto set_ttl(int sockfd, unsigned  char ttl){
    check(setsockopt(sockfd, SOL_IP, IP_TTL, &ttl, sizeof(ttl)));
}

union {
    unsigned char buffer[IP_MAXPACKET];
    struct {
        iphdr ip_hdr;
        icmphdr icmp_hdr;
        iphdr prev_ip_hdr;              // only valid for some ICMP Types (e.g. Destination Unreachable or Time Expired)
        icmphdr prev_icmp_hdr;          // same as above
    };
} input_buffer;


enum class Result{
    Found, Unreachable, TtlExpired, Ignore
};

Result check_response(const sockaddr_in& source_addr, const icmphdr& last_request)
{
    auto in_type = input_buffer.icmp_hdr.type;
    if(source_addr.sin_addr.s_addr == TARGET_ADDR
       && in_type == ICMP_ECHOREPLY
       && ntohs(input_buffer.icmp_hdr.un.echo.id) == ID
       && input_buffer.icmp_hdr.un.echo.sequence == last_request.un.echo.sequence)
        return Result::Found;                                // it is response from our target to our request
    if(in_type != ICMP_TIME_EXCEEDED && in_type != ICMP_DEST_UNREACH)
        return Result::Ignore;                               // not an expected code, no reaction

    if(input_buffer.prev_ip_hdr.protocol != IPPROTO_ICMP)
        return Result::Ignore;                               // this ICMP message is not a reaction to an ICMP message

    if(input_buffer.prev_icmp_hdr.type != ICMP_ECHO)         // this ICMP message is not a reaction to an ICMP Echo message
        return Result::Ignore;

    if(input_buffer.prev_ip_hdr.daddr != TARGET_ADDR)        // this ICMP message is not a reaction to an ICMP message for our target
        return Result::Ignore;

    auto prev_id = ntohs(input_buffer.prev_icmp_hdr.un.echo.id);
    if(prev_id != ID)
        return Result::Ignore;                               // this ICMP message is not a reaction to our ICMP message

    if(in_type == ICMP_DEST_UNREACH)
        return Result::Unreachable;
    auto prev_seq_num = input_buffer.prev_icmp_hdr.un.echo.sequence;
    auto current_seq_num = last_request.un.echo.sequence;
    if (prev_seq_num != current_seq_num)
        return Result::Ignore;                               // this ICMP message is a reaction for the old message, consider lost
    return Result::TtlExpired;
}

const char DEFAULT_ADDR[] = "8.8.8.8"; // Google DNS
int main(int argc, char** argv){
    const char* target_addr_str = DEFAULT_ADDR;
    if(argc > 1)
        target_addr_str = argv[1];
    TARGET_ADDR = inet_addr(target_addr_str);
    if(TARGET_ADDR == INADDR_NONE){
        std::cerr << "Wrong target address "<< TARGET_ADDR;
        exit(EXIT_FAILURE);
    }
    std::cout << "Searching route to "<<target_addr_str<<std::endl;

    uint16_t sequence_number = 1;
    auto sockfd = make_icmp_socket();
    const sockaddr_in target_addr = {.sin_family=AF_INET,.sin_port = 0,.sin_addr = {TARGET_ADDR} };

    Result result = Result::Ignore;
    for(unsigned char ttl = 1; ttl < MAX_HOPS; ++ttl){
        auto out_hdr = create_echo_request(ID, ++sequence_number);
        set_ttl(sockfd, ttl);
        check(sendto(sockfd, &out_hdr, sizeof(out_hdr), 0,
                     (const sockaddr *)(&target_addr), sizeof(target_addr)));

        using clock = std::chrono::system_clock;
        auto send_time = clock::now();

        sockaddr_in addr{};
        result = Result::Ignore;
        while(true){
            socklen_t len = sizeof addr;
            auto answer_size = check_except(recvfrom(sockfd,
                                                     &input_buffer, sizeof input_buffer,
                                                     MSG_DONTWAIT,                          // do not block
                                                     (sockaddr*)(&addr), &len),
                                            EWOULDBLOCK, EAGAIN);

            if(answer_size >0)
                result = check_response(addr, out_hdr);
            if(result != Result::Ignore)
                break;
            if(clock::now() - send_time > WAIT_TIME)
                break;
        }
        if(result != Result::Ignore){
            std::cout << (int)ttl << "\t"<< inet_ntoa(addr.sin_addr)  << "\t";
            switch (result) {
                case Result::Unreachable: std::cout << "No route to target"; break;
                case Result::Found: std::cout << " Found!"; break;
                default:  break;
            }
            std::cout << std::endl;
        }
        else{
            std::cout << (int)ttl << "\t???\t" <<  std::endl;
        }
        if(result != Result::Ignore && result!=Result::TtlExpired)
            break;
    }
    if(result == Result::Ignore)
        std::cout << "Target was not found. Try increase wait time"<<std::endl;
    if(result  == Result::TtlExpired)
        std::cout << "Target was not found. Max TTL " << (int)MAX_HOPS << "is insufficient"<<std::endl;
}