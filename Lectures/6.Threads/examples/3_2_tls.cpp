#include "common.hpp"
#include <pthread.h>
#include <string_view>
#include <string.h>
#include <vector>
#include <syncstream>

#define COUT std::osyncstream(std::cout)

struct NamedObject{
    const std::string name;
    int value = 0;

    explicit NamedObject(std::string_view object_name): name(object_name){
        auto time = get_current_time();
        COUT << "Object "<<std::quoted(name) << " created at "<< time << std::endl;
    }

    ~NamedObject(){
        auto time = get_current_time();
        COUT << "Object "<<std::quoted(name) << " destroyed at "<< time
                  << " with value "<< value << std::endl;
    }
};

NamedObject global = NamedObject{"Global"};
pthread_key_t thread_local_key;


void destroy_named_object(void* obj){
    delete (NamedObject*)obj;
}

NamedObject* get_thread_local(){
    auto thread_local_ = (NamedObject*)pthread_getspecific(thread_local_key);
    if(thread_local_ == nullptr)
    {
        thread_local_ = new NamedObject("Thread-local");
        check_result(pthread_setspecific(thread_local_key, thread_local_));
    }
    return thread_local_;
}

void* thread_fn(void* arg_){
    int* arg = (int*)arg_;
    auto thread_local_ = get_thread_local();

    ++thread_local_->value;
    ++global.value;
    timespec ts{.tv_sec = 0, .tv_nsec = 300000000};
    nanosleep(&ts, nullptr);
    return nullptr;
};




int main(){

    check_result(pthread_key_create(&thread_local_key, destroy_named_object));


    global.value = 0;

    size_t thread_count;
    while(true){
        COUT << "Threads count:";
        std::cin >> thread_count;
        if(std::cin.good()) break;
    }

    std::vector<pthread_t> threads;
    for(size_t i = 0; i < thread_count; ++i)
    {
        pthread_t tid;
        check_result(pthread_create(&tid, nullptr, thread_fn, nullptr));
        threads.push_back(tid);
    }

    for(const auto& tid: threads)
        check_result(pthread_join(tid, nullptr));

}