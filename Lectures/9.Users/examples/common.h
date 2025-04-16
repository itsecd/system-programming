//
// Created by Alexey on 10.04.2024.
//

#ifndef LECTURE9_COMMON_H
#define LECTURE9_COMMON_H
#include "check.hpp"
#include <unistd.h>
#include <iostream>

inline std::ostream& current_ids(std::ostream& s){
    uid_t rid, eid, sid;
    check(getresuid(&rid, &eid, &sid));
    return s << "(rUID="<<rid << " eUID=" << eid << " sUID=" <<sid <<")";
}

#endif //LECTURE9_COMMON_H
