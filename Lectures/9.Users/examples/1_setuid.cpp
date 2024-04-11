#include <unistd.h>
#include "common.h"
#include <pwd.h>

uid_t get_current_user_id(){
    const char* username = getenv("SUDO_USER");
    if(!username)
        return getuid();
    const passwd* user_info = getpwnam(username);
    return user_info ? user_info->pw_uid : getuid();
}

uid_t get_nonexistent_uid(){
    uid_t id =11111;
    while(true){
        const passwd* user_info = getpwuid(id);
        if(!user_info)
            return id;
        ++id;
    }
}

uid_t get_some_uid(){
    uid_t id =1;
    while(true){
        const passwd* user_info = getpwuid(id);
        if(user_info)
            return id;
        ++id;
    }
}


int main(int argc, char** argv){
    std::cout << "Run this program with and without sudo, run with set-user-id flag, and compare the output."<< std::endl;
    std::cout << std::endl << "Initial IDs "<< std::endl;
    std::cout << current_ids << std::endl<< std::endl;
    uid_t nonexistent = get_nonexistent_uid();
    uid_t current = get_current_user_id();
    uid_t real = get_some_uid();

    std::cout << "Setting eUID to " << current;
    if(check_except(seteuid(current), EPERM))
        std::cout << "FAILED";
    std::cout << std::endl << current_ids << std::endl<< std::endl;

    std::cout << "Resetting eUID to UID ";
    if(check_except(seteuid(getuid()), EPERM))
        std::cout << "FAILED";
    std::cout << std::endl << current_ids << std::endl<< std::endl;

    std::cout << "Setting eUID to "<<real
              << " which IS listed in passwd ";
    if(check_except(seteuid(real), EPERM))
        std::cout << "FAILED";
    std::cout << std::endl << current_ids << std::endl<< std::endl;

    std::cout << "Resetting eUID to UID ";
    if(check_except(seteuid(getuid()), EPERM))
        std::cout << "FAILED";
    std::cout << std::endl << current_ids << std::endl<< std::endl;

    std::cout << "Setting eUID to "<<nonexistent
                        << " which IS NOT listed in passwd ";
    if(check_except(seteuid(nonexistent), EPERM, EINVAL))
        std::cout << "FAILED";
    std::cout << std::endl << current_ids << std::endl<< std::endl;

    std::cout << "Resetting eUID to UID ";
    if(check_except(seteuid(getuid()), EPERM))
        std::cout << "FAILED";
    std::cout << std::endl << current_ids << std::endl<< std::endl;

retry:
    std::cout << "Setting eUID to " << current << " using setuid() " ;
    if(check_except(setuid(current), EPERM))
        std::cout << "FAILED";
    std::cout << std::endl << current_ids << std::endl<< std::endl;

    std::cout << "Setting eUID to 0 (root) " <<std::endl;
    if(check_except(seteuid(0), EPERM)){
        std::cout << "Failed to regain root privileges" << std::endl;
        std::cout <<  current_ids << std::endl<< std::endl;
    }
    else{
        std::cout << "Regained the root privileges (possibly because of saved UID) " << std::endl;
        std::cout <<  current_ids << std::endl<< std::endl;
        goto retry;  // :)
    }
}