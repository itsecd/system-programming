#ifndef LECTURE4_COMMON_H
#define LECTURE4_COMMON_H

#include <signal.h>
#include <stdio.h>

inline const char* signal_name(int s){
#ifdef __APPLE__
    return strsignal(s);
#else
    const char* name =  sigabbrev_np(s);
    if (!name)
        name = strsignal(s);
    return name;
#endif
}

inline void default_handler(int s){
        printf("----handler: %d (%s)\n", s, signal_name(s));
}

#endif //LECTURE4_COMMON_H
