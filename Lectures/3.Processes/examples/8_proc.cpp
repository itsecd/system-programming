#include <stdio.h>
#include <unistd.h>
volatile int global = 6311;

int main(){
    volatile int local = 6312;
    auto pid = getpid();
    printf("local variable is at address %p \n", &local);
    printf("global variable is at address %p \n", &global);


    printf("local variable value is %d\n", local);
    printf("global variable value is %d\n", global);

    printf("To read value, use command: sudo dd bs=1 skip=$((<address>)) count=4 if=/proc/%d/mem status=none | hexdump -e '1/4 \"%%04d\" \"\\n\"' \n", pid);
    printf("To write a random value, use command: sudo  dd bs=1 seek=$((<address>)) count=4 if=/dev/urandom of=/proc/%d/mem conv=notrunc status=none \n", pid);

    printf("Enter anything to proceed\n");
    getchar();


    printf("local variable value is %d\n", local);
    printf("global variable value is %d\n", global);


}
