#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "check.hpp"
#include <string.h>
#include <chrono>
#include <iostream>
#include <sys/stat.h>


using namespace std::chrono;

struct User{
    int age;
    char name[16];

};

std::ostream& operator<<(std::ostream& s,const User& u){
    char name[17]{};
    strncpy(name, u.name, sizeof u.name);
    return s << "{ " << name << "  age " << u.age << " }";
}


std::string make_temp_file(){
    User users[16];
    for(auto& u: users){
        u.age = rand() % 100;
        auto name = "User "+std::to_string(rand() % 1000);
        strncpy(u.name, name.c_str(), sizeof u.name);
    }
    char fname[] = "/tmp/usersXXXXXX";
    int fd = check(mkstemp(fname));

    auto written = check(write(fd, users, sizeof users));
    if( written < sizeof users){
        printf("Failed to write the full users array to %s. Only %ld bytes were written", fname, written);
        exit(2);
    }
    return fname;
}


void read_regular(std::string_view fname){
    std::cout << "Reading using read(): ";
    int fd = check(open(fname.data(), O_RDONLY));
    User u{};
    while(true){
        auto read_size = check(read(fd, &u, sizeof u));
        if(read_size < sizeof u) break; // end-of-file
        std::cout << u << " ";
    }
    std::cout << std::endl;
}

size_t get_file_size(int fd){
    struct stat s{};
    check(fstat(fd, &s));
    return s.st_size;
}

void read_mmap(std::string_view fname){
    std::cout << "Reading using mmap(): ";
    int fd = check(open(fname.data(), O_RDONLY));
    size_t size = get_file_size(fd);

    User* users = (User*)check(mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0));
    for(int i = 0; i < size / sizeof(User); ++i)
        std::cout << users[i] << " ";

    munmap(users, size);
    std::endl(std::cout);
}

int main(){
    //srand(0);
    auto fname = make_temp_file();
    std::cout << "Created file "<< fname << std::endl;

    read_regular(fname);
    read_mmap(fname);
}