#include "common.hpp"
#include <pthread.h>
#include <string_view>
#include <string.h>
#include <vector>

struct NamedObject{
    const char* const name;
    int value;
};

NamedObject* global;
pthread_key_t thread_global_key;

NamedObject* make_named_object(const char* name){
    auto name_size = strlen(name)+1;
    char * name_copy = new char [name_size];
    memcpy(name_copy, name, name_size);
    auto result = new NamedObject{name_copy, 0};
    auto time = get_current_time();
    std::cout << "Object "<<std::quoted(result->name) << " created at "<< time << std::endl;
    return result;
}

void destroy_named_object(void* obj_){
    if(!obj_) return;
    auto obj = (NamedObject*)obj_;
    auto time = get_current_time();
    std::cout << "Object "<<std::quoted(obj->name) << " destroyed at "<< time
              << " with value "<< obj->value << std::endl;
    delete[] obj->name;
    delete obj;
}

NamedObject* get_thread_global(){
    NamedObject* thread_global = (NamedObject*)pthread_getspecific(thread_global_key);
    if( thread_global == nullptr)
    {
        thread_global = make_named_object("Thread-local");
        check_result(pthread_setspecific(thread_global_key, thread_global));
    }
    return thread_global;
}

void* thread_fn(void* arg_){

    auto thread_global = get_thread_global();
    int* arg = (int*)arg_;
    thread_global->value = *arg;
    global->value = *arg;
    timespec ts{.tv_sec = 0, .tv_nsec = 300000000};
    nanosleep(&ts, nullptr);
    return nullptr;
};




int main(){

    check_result(pthread_key_create(&thread_global_key, destroy_named_object));
    global = make_named_object("Global");

    auto thread_global = get_thread_global();

    global->value = -1;
    thread_global->value  = -1;

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

    destroy_named_object(global);
}