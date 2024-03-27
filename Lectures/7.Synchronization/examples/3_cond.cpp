#include <iostream>
#include "common.hpp"

const size_t CONSUMERS_COUNT = 2;
const size_t CONSUMER_LIMIT =100000;

int value;
bool ready;
uint64_t reader_empty_iterations = 0;
uint64_t writer_empty_iterations = 0;
pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ready_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_ready_cond = PTHREAD_COND_INITIALIZER;



void* consume_fn(void* arg){
    size_t counter = 0;
    int64_t result = 0;
    while(counter < CONSUMER_LIMIT) {
        check_result(pthread_mutex_lock(&mutex));
        if(ready)
        {
            result += value;
            ready = false;
            ++counter;
        }
        else
            ++reader_empty_iterations;
        pthread_mutex_unlock(&mutex);
    }
    if(arg){ *(int64_t *)arg = result;}
    return nullptr;
}

void* produce_fn(void*){

    size_t counter = 0;
    while(counter < CONSUMERS_COUNT*CONSUMER_LIMIT) {
        check_result(pthread_mutex_lock(&mutex));
        if(!ready)
        {
            value = rand();
            ready = true;
            ++counter;
        }
        else
            ++reader_empty_iterations;
        pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}


void* consume_fn2(void* arg){
    size_t counter = 0;
    size_t result = 0;
    while(counter < CONSUMER_LIMIT) {
        check_result(pthread_mutex_lock(&mutex));

        while(!ready) {

            check_result(pthread_cond_wait(&ready_cond, &mutex));
            if(!ready) ++writer_empty_iterations;
        }

        result += value;
        ready = false;
        ++counter;
        pthread_cond_signal(&not_ready_cond);
        pthread_mutex_unlock(&mutex);
    }
    if(arg){ *(int64_t *)arg = result;}
    return nullptr;
}

void* produce_fn2(void*){
    size_t counter = 0;
    while(counter < CONSUMERS_COUNT*CONSUMER_LIMIT) {
        check_result(pthread_mutex_lock(&mutex));
        while(ready){

            check_result(pthread_cond_wait(&not_ready_cond, &mutex));
            if(ready) ++writer_empty_iterations;
        }

        value = rand();
        ready = true;
        ++counter;
        pthread_cond_signal(&ready_cond);
        pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}


int main(){

    reader_empty_iterations = 0;
    writer_empty_iterations = 0;
    {
        ScopedTimer s{"Default"};
        auto producer = spawn_threads(1, produce_fn);
        join_threads(spawn_threads(CONSUMERS_COUNT, consume_fn));
        join_threads(producer);
    }
    std::cout << "Reader empty iterations: " << reader_empty_iterations <<std::endl;
    std::cout << "Writer empty iterations: " << writer_empty_iterations <<std::endl;

    reader_empty_iterations = 0;
    writer_empty_iterations = 0;
    std::cout << std::endl;
    {
        ScopedTimer s{"With condition var"};
        auto producer = spawn_threads(1, produce_fn2);
        join_threads(spawn_threads(CONSUMERS_COUNT, consume_fn2));
        join_threads(producer);
    }
    std::cout << "Reader empty iterations: " << reader_empty_iterations <<std::endl;
    std::cout << "Writer empty iterations: " << writer_empty_iterations <<std::endl;
}
