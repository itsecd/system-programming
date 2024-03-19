#include "common.hpp"
#include <pthread.h>
#include <iostream>
#include <vector>
#include <cmath>


struct ThreadArg{
    double a, b, c;
};

std::ostream& operator<<(std::ostream &s, const ThreadArg& obj){
    return s << "{ a: "<<obj.a<< " , b: "<<obj.b<<" , c: "<<obj.c<<" }";
}

struct ThreadResult{
    double x1, x2;
};

std::ostream& operator<<(std::ostream &s, const ThreadResult& r){
    return s << "{ x1: " << r.x1 << " , x2: "<< r.x2 << "}";
}


void* thread_main(void* arg_){

    auto arg = (ThreadArg*)arg_;

    double d = (arg->b)*(arg->b) -4*arg->a*arg->c;
    double x1 = (-arg->b+sqrt(d) )/(2*arg->a);
    double x2 = (-arg->b-sqrt(d) )/(2*arg->a);
    delete arg;

    return new ThreadResult{x1, x2};
}


int main(){

    size_t thread_num;
    while(true){
        std::cout << "Threads count: ";
        std::cin >> thread_num;
        if(std::cin.good()) break;
    }
    std::vector<pthread_t > threads;

    for(size_t i = 0; i < thread_num; ++i){
        auto arg = new ThreadArg{(double)1, 2.0*i, -(double)i*i};

        pthread_t tid;
        check_result(pthread_create(&tid, nullptr, thread_main, arg));
        auto current_time = get_current_time();
        std::cout << "Thread " << i << " with args "<< *arg<< " created at " << current_time << std::endl;

        threads.push_back(tid);
    }

    for(size_t i = 0; i < thread_num; ++i){
        ThreadResult* result;
        check_result(pthread_join(threads[i], (void**)&result));
        std::cout << "Result of thread " << i << " is " << *result << std::endl;
    }

}