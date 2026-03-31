#include <iostream>
#include "common.hpp"

constexpr size_t CONSUMERS_COUNT = 4;
constexpr size_t VALUES_PER_CONSUMER = 81920;
constexpr nanoseconds CONSUMING_DELAY = 100;

struct {
    pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;
    int value;
    bool has_value;
} common_resource;

pthread_cond_t ready_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_ready_cond = PTHREAD_COND_INITIALIZER;

uint64_t consumer_empty_iterations;
uint64_t producer_empty_iterations;

void simulate_consuming()
{
    delay(CONSUMING_DELAY);
}


void* consume_fn(void* arg){
    size_t counter = 0;
    int64_t result = 0;

    while(counter < VALUES_PER_CONSUMER) {
        bool have_value = false;
        int value;

        check_result(pthread_mutex_lock(&common_resource.mutex)); // CRITICAL SECTION
        {
            if(common_resource.has_value)
            {
                value = common_resource.value;
                common_resource.has_value = false;
                have_value = true;
            }
            else
                ++consumer_empty_iterations;
        }
        pthread_mutex_unlock(&common_resource.mutex);             //~CRITICAL SECTION

        if (have_value)
        {
            result += value;
            ++counter;
            simulate_consuming();
        }
    }
    if(arg){ *(int64_t *)arg = result;}
    return nullptr;
}

void* produce_fn(void*){

    size_t counter = 0;

    while(counter < CONSUMERS_COUNT * VALUES_PER_CONSUMER) {
        check_result(pthread_mutex_lock(&common_resource.mutex));
        if(!common_resource.has_value)
        {
            common_resource.value = rand();
            common_resource.has_value = true;
            ++counter;
        }
        else
            ++producer_empty_iterations;
        pthread_mutex_unlock(&common_resource.mutex);

    }
    return nullptr;
}


void* consume_fn2(void* arg){
    size_t counter = 0;
    int64_t result = 0;
    while(counter < VALUES_PER_CONSUMER) {
        int value;

        check_result(pthread_mutex_lock(&common_resource.mutex)); // CRITICAL SECTION
        {
            while(!common_resource.has_value) {
                check_result(pthread_cond_wait(&ready_cond, &common_resource.mutex));
                if(!common_resource.has_value) ++consumer_empty_iterations;
            }

            value = common_resource.value;
            common_resource.has_value = false;
            pthread_cond_signal(&not_ready_cond);
        }
        pthread_mutex_unlock(&common_resource.mutex);             //~CRITICAL SECTION

        ++counter;
        result += value;
        simulate_consuming();
    }
    if(arg){ *(int64_t *)arg = result;}
    return nullptr;
}

void* produce_fn2(void*){
    size_t counter = 0;
    while(counter < CONSUMERS_COUNT * VALUES_PER_CONSUMER) {
        check_result(pthread_mutex_lock(&common_resource.mutex));
        {
            while(common_resource.has_value){
                check_result(pthread_cond_wait(&not_ready_cond, &common_resource.mutex));
                if(common_resource.has_value) ++producer_empty_iterations;
            }

            common_resource.value = rand();
            common_resource.has_value = true;
            ++counter;
            pthread_cond_signal(&ready_cond);
        }
        pthread_mutex_unlock(&common_resource.mutex);
    }
    return nullptr;
}


void run(const thread_fn producer_fn, const thread_fn consumer_fn)
{
    consumer_empty_iterations = 0;
    producer_empty_iterations = 0;
    common_resource = {};
    auto consumers = spawn_threads( consumer_fn, CONSUMERS_COUNT);
    producer_fn(nullptr);
    join_threads(consumers);
    std::cout << "Consumer empty iterations: " << consumer_empty_iterations <<std::endl;
    std::cout << "Producer empty iterations: " << producer_empty_iterations <<std::endl;

}

int main(){
    srand(0);
    {
        ScopedTimer s{"Mutex-only"};
        run(produce_fn, consume_fn);
    }
    std::cout << std::endl;
    {
        ScopedTimer s{"Mutex+Condition variable"};
        run(produce_fn2, consume_fn2);
    }
}
