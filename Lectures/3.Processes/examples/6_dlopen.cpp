#include <dlfcn.h>
#include "check.hpp"

extern "C" void foo(){
    puts("Foo :) ");

}


int main(){
    dlerror();
    void* global_handle = dlopen(NULL, RTLD_NOW);

    void* foo_ptr = check(dlsym(global_handle, "foo"));

    printf("Function foo() is at %p\n", foo_ptr);
    puts("Calling foo by pointer");
    ( (void(*)()) foo_ptr)();

    void* libc_handle = dlopen("libc.so.6", RTLD_NOW);
    if(!libc_handle){
        fprintf(stderr, "%s",  dlerror());
        exit(-1);
    }

    int(* puts_ptr )(const char*)  = (int(*)(const char*))dlsym(libc_handle, "puts");
    if(!puts_ptr ){
        fprintf(stderr, "Failed to get puts() address: %s\n",  dlerror());
        exit(-2);
    }
    puts_ptr("Calling puts() by pointer");

}