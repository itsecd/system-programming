#include "common.hpp"
#include <pthread.h>
#include <string_view>
#include <vector>


struct NamedObject{
    const std::string name;
    int value = 0;

    NamedObject(std::string_view object_name): name(object_name){
        auto time = get_current_time();
        std::cout << "Object "<<std::quoted(name) << " created at "<< time << std::endl;
    }

    ~NamedObject(){
        auto time = get_current_time();
        std::cout << "Object "<<std::quoted(name) << " destroyed at "<< time
                                                                        << " with value "<< value << std::endl;
    }
};


NamedObject global{"Global"};


thread_local  NamedObject thread_global{"Thread-local"}; // created only if there is any operation with this object


void* thread_fn(void* arg_){
    int* arg = (int*)arg_;
    thread_global.value = *arg;
    global.value = *arg;
    timespec ts{.tv_sec = 0, .tv_nsec = 300000000};
    nanosleep(&ts, nullptr); // imitate some work
    delete arg;
    return nullptr;
};

int main(){
    thread_global.value = -1;
    global.value = -1;
    size_t thread_count;
    while(true){
        std::cout << "Threads count:";
        std::cin >> thread_count;
        if(std::cin.good()) break;
    }

    std::vector<pthread_t> threads;
    for(size_t i = 0; i < thread_count; ++i)
    {
        pthread_t tid;
        check_result(pthread_create(&tid, nullptr, thread_fn, new int(i)));
        threads.push_back(tid);
    }

    for(const auto& tid: threads)
        check_result(pthread_join(tid, nullptr));
}