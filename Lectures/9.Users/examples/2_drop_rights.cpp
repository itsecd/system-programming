#include "common.h"
#include <fcntl.h>
#include <iostream>
#include <string>
#include <memory>

std::string get_exe_path(){
    const size_t buf_size = 4097;
    std::unique_ptr<char[]> buf { new char [buf_size] };
    check(readlink("/proc/self/exe", buf.get(), buf_size));
    return {buf.get() };
}

int main(){
    int count = 0;
retry:
    std::cout << "Current UIDs: " << current_ids << std::endl;

    ++count;
    std::cout << "Opening /dev/mem" << std::endl;
    int fd = check_except(open("/dev/mem", O_RDONLY), EACCES);
    if(fd < 0)
    {
        std::cerr << "Failed to open /dev/mem. Not a root" << std::endl;
        if(count < 2 && geteuid() == getuid()) // no set-user-id flag was set
        {
            auto exe_path = get_exe_path();
            std::cout << "To change the program owner to root and set set-uset-id flag use commands"<<std::endl;
            std::cout << "  sudo chown root:root "<<exe_path<<std::endl;
            std::cout << "  sudo chmod u+s "<<exe_path<<std::endl;
        }
        exit(0);
    }
    std::cout << "Reading first 8 bytes of /dev/mem: ";
    long long val;
    check(read(fd, &val,sizeof val));
    std::cout << std::hex << val<<std::dec <<std::endl;
    std::cout << std::endl<< "Dropping root rights"<< std::endl << std::endl;
    check(setuid(getuid()));
    std::cout << "Reading second 8 bytes of /dev/mem: ";
    check(read(fd, &val,sizeof val));
    std::cout << std::hex << val<<std::dec <<std::endl;
    std::cout << "Closing descriptor" << std::endl;
    close(fd);
    if(count < 2)
        goto retry;

}