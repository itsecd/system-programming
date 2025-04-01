#include "common.hpp"

const size_t CONSUMERS_COUNT = 8;
const size_t VALUES_PER_CONSUMER =100000;

int value;
bool ready;

pthread_mutex_t  mutex ;
pthread_spinlock_t spin;

void* spin_consume_fn(void* arg){
    size_t counter = 0;
    size_t result = 0;
    while(counter < VALUES_PER_CONSUMER) {
        check_result(pthread_spin_lock(&spin));
        if(ready)
        {
            result += value;
            ready = false;
            ++counter;
        }
        pthread_spin_unlock(&spin);
    }
    if(arg != nullptr){ *(size_t*)arg = result;}
    return nullptr;
}

void* spin_produce_fn(void*){
    size_t counter = 0;
    while(counter < CONSUMERS_COUNT * VALUES_PER_CONSUMER) {
        check_result(pthread_spin_lock(&spin));
        if(!ready)
        {
            value = rand();
            ready = true;
            ++counter;
        }
        pthread_spin_unlock(&spin);
    }
    return nullptr;
}

void* mutex_consume_fn(void* arg){
    size_t counter = 0;
    size_t result = 0;
    while(counter < VALUES_PER_CONSUMER) {
        check_result(pthread_mutex_lock(&mutex));
        if(ready)
        {
            result += value;
            ready = false;
            ++counter;
        }
        pthread_mutex_unlock(&mutex);
    }
    if(arg != nullptr){ *(size_t*)arg = result;}
    return nullptr;
}

void* mutex_produce_fn(void*){
    size_t counter = 0;
    while(counter < CONSUMERS_COUNT * VALUES_PER_CONSUMER) {
        check_result(pthread_mutex_lock(&mutex));
        if(!ready)
        {
            value = rand();
            ready = true;
            ++counter;
        }
        pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}


int main(){
    check_result(pthread_mutex_init(&mutex, nullptr));
    check_result(pthread_spin_init(&spin, false));

    {
        ScopedTimer s{"Spin-lock"};
        auto producer = spawn_threads(1, spin_produce_fn);
        join_threads(spawn_threads(CONSUMERS_COUNT, spin_consume_fn));
        join_threads(producer);
    }

    {
        ScopedTimer s{"Mutex"};
        auto producer = spawn_threads(1, mutex_produce_fn);
        join_threads(spawn_threads(CONSUMERS_COUNT, mutex_consume_fn));
        join_threads(producer);

    }


}
