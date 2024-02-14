#include <fcntl.h>
#include "sys/ioctl.h"
#include "linux/rtc.h"
#include "check.hpp"
#include <unistd.h>


void print_time(const rtc_time& time){
    printf("%dy %dm %dd %dh %dm %ds\n",
           1900+time.tm_year, time.tm_mon+1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
}




int main(){
    if(getuid() !=0 )
    {
        fputs("Program must be run with sudo (needs access to /dev/rtc). Exiting", stderr);
        exit(-1);
    }

    int fd = check(open("/dev/rtc", O_RDONLY));


    rtc_time time{};
    fputs("Current time:", stdout);
    check(ioctl(fd, RTC_RD_TIME, &time));
    print_time(time);

    puts("Setting alarm 15 seconds ahead");
    fflush(stdout);

    time.tm_sec+= 15;
    if(time.tm_sec >= 60){
        time.tm_sec %= 60;
        ++time.tm_min;
    }
    if(time.tm_min >= 60){
        time.tm_min %=60;
        time.tm_hour = (time.tm_hour+1)%24;
    }
    check(ioctl(fd, RTC_ALM_SET, &time));        //Set time
    check(ioctl(fd, RTC_AIE_ON));                //Enable alarm interrupt

    long buf;
    check(read(fd, &buf, sizeof buf));            // read will block until alarm

    fputs("Alarm rang at:", stdout);
    check(ioctl(fd, RTC_RD_TIME, &time));
    print_time(time);
}