#ifndef LECTURE5_MESSAGE_HPP
#define LECTURE5_MESSAGE_HPP

#include <iostream>
#include <signal.h>
#include "check.hpp"

#define COUT (std::cout << getpid() << ">> ")


struct Message{
    int x;
    char str[256];
};

inline std::ostream&  operator << (std::ostream& s, const Message& m){
    return s << "Message { x : " <<m.x << ",  str: '" << &m.str[0] << "'}"<<std::endl;
}

inline void read_message( Message& m){
    constexpr auto max_size = std::numeric_limits<std::streamsize>::max();

    while(true){
        COUT << "Value of X: "<<std::flush;
        std::cin >> m.x;
        if(std::cin) break;
        if(std::cin.bad() || std::cin.eof()) exit(-2);
        std::cin.clear();
        std::cin.ignore(max_size, '\n');
    }

    std::string str;
    COUT << "Value of MSG:"<<std::flush;
    std::cin >> str;
    size_t size = std::min(str.size(), sizeof m.str -1);
    std::copy(str.c_str(), str.c_str()+size, &m.str[0]);
    m.str[size] = 0;
}


inline void ask_continue(){
    COUT << "Continue? " << std::endl;
    std::string str;
    std::cin >> str;
    if( tolower(str[0]) != 'y') exit(0);
}


#endif //LECTURE5_MESSAGE_HPP
