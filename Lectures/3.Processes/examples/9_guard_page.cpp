#include "check.hpp"
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <memory.h>
const int PAGE_SIZE = 4096;


void* guarded_alloc(size_t size){
    size_t alloc_size = ((size+PAGE_SIZE -1)/PAGE_SIZE)*PAGE_SIZE; // round up
    alloc_size += PAGE_SIZE;
    void* result = check(mmap(nullptr, alloc_size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0));
    check(mprotect(result, alloc_size - PAGE_SIZE, PROT_READ|PROT_WRITE));  // change protection for all pages except last;

    result = (char*)result+alloc_size - PAGE_SIZE - size;   // return pointer to area adjacent to the guard page
    return result;
};

void guarded_dealloc(void* ptr, size_t size){
    size_t alloc_size = (size+PAGE_SIZE -1)/PAGE_SIZE; // round up
    alloc_size += PAGE_SIZE;
    ptr = (void*)((long long)ptr & -(PAGE_SIZE)); //align memory to a page boundary
    check(munmap(ptr, alloc_size));
}


const size_t SIZE = 16;
void common(){
    char* buf = (char*)calloc(SIZE, 1);
    char* buf2 = (char*)calloc(SIZE, 1);
    scanf("%s", buf);
    printf("%s\n", buf);
    printf("%s\n", buf2);
    free(buf);
    free(buf2);
}

void guarded(){
    char* buf = (char*)guarded_alloc(SIZE);
    char* buf2 = (char*)guarded_alloc(SIZE);

    scanf("%s", buf);
    printf("%s\n", buf);
    printf("%s\n", buf2);

    guarded_dealloc(buf, SIZE);
    guarded_dealloc(buf2, SIZE);

}



int main(int argc, char** argv){
    if(argc > 1) {
        puts("Using guard pages");
        guarded();
    }
    else
        common();
}