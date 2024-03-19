//
// Created by alexey on 19.03.24.
//

#ifndef LECTURE6_COMMON_HPP
#define LECTURE6_COMMON_HPP
#include <time.h>
#include "check.hpp"

inline timespec get_current_time(){
    timespec t{};
    check(clock_gettime(CLOCK_REALTIME, &t));
    return t;
}

inline std::ostream& operator<<(std::ostream& s, const timespec& t){
    tm result{};
    localtime_r(&t.tv_sec, &result);
    return s << std::put_time(&result, "%d/%m/%y %T.")
                            << std::setfill('0') << std::setw(6) << t.tv_nsec / 1000;
}



#endif //LECTURE6_COMMON_HPP
