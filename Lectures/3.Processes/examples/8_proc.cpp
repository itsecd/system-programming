#include <stdio.h>
#include <unistd.h>
volatile int global = 0x6311;

int main(){
    volatile int local = 0x6312;
    auto pid = getpid();
    printf("local variable is at address %p \n", &local);
    printf("global variable is at address %p \n\n", &global);


    printf("local variable value is %x\n", local);
    printf("global variable value is %x\n\n", global);
    puts("[LOCAL]");
    printf("Command to read: \n\t dd bs=1 skip=$((%p)) count=4 if=/proc/%d/mem status=none | hexdump -e '1/4 \"%%04d\" \"\\n\"' \n", &local, pid);
    printf("Command to write a random value: \n\t dd bs=1 seek=$((%p)) count=4 if=/dev/urandom of=/proc/%d/mem conv=notrunc status=none \n\n", &local, pid);

    puts("[GLOBAL]");
    printf("Command to read: \n\t dd bs=1 skip=$((%p)) count=4 if=/proc/%d/mem status=none | hexdump -e '1/4 \"%%04d\" \"\\n\"' \n", &global, pid);
    printf("Command to write a random value: \n\t dd bs=1 seek=$((%p)) count=4 if=/dev/urandom of=/proc/%d/mem conv=notrunc status=none \n\n", &global, pid);


    printf("Enter anything to proceed\n");
    getchar();


    printf("local variable value is %x\n", local);
    printf("global variable value is %x\n", global);


}
