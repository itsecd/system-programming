#include "common.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

#define ARR_SIZE 12
static_assert(ARR_SIZE > 0);

bool is_sorted(int* arr, size_t size){
    if(size < 2)
        return true;
    if(arr[0] > arr[1])
        return false;
    return is_sorted(++arr, --size);

}

void shuffle(int* arr, size_t size){
    for(size_t i = 0; i < size; ++i)
    {
        size_t idx = rand() %size;
        std::swap(arr[i], arr[idx]);
    }
}

struct Arg{
    int* ptr;
    size_t size;
};

void* work_thread_fn(void* arg_){
    Arg* arg = (Arg*)arg_;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, nullptr);

    while(!is_sorted(arg->ptr, arg->size)) {
        shuffle(arg->ptr, arg->size);
    }

    auto result = new timespec();
    *result = get_current_time();
    return result;
}

void* wait_thread_fn(void* arg_){
    const auto* arg = (pthread_t*)arg_;

    std::cout << "Enter anything to stop " << std::flush ;
    getchar_unlocked();   // getchar() may block forever
    pthread_cancel(*arg);
    return nullptr;
}

void print_array(int* arr, size_t size){
    std::cout << "{ ";
    for(int i = 0; i < size; ++i )
        std::cout << arr[i] << " ";
    std::cout << "}";
}

int main(){

    int array[ARR_SIZE];

    for(auto& v: array) v = rand() % 1000;

    std::cout << "Initial state: ";
    print_array(array, ARR_SIZE);
    std::cout << std::endl;

    Arg arg { array, ARR_SIZE};

    pthread_t working_thread, waiting_thread;

    check_result(pthread_create(&working_thread, nullptr, work_thread_fn, &arg));
    auto time = get_current_time();
    std::cout << "Working thread started at "  << time << std::endl;

    check_result(pthread_create(&waiting_thread, nullptr, wait_thread_fn, &working_thread));
    check_result(pthread_detach(waiting_thread));

    timespec* end_time;
    check_result(pthread_join(working_thread, (void**)&end_time));

    std::cout << std::endl;
    if(end_time == PTHREAD_CANCELED)
        std::cout << "Working thread was cancelled" << std::endl;
    else {
        std::cout << "Working thread completed at " << *end_time << std::endl;
        delete end_time;
    }

    std::cout << "Final state: ";
    print_array(array, ARR_SIZE);
}