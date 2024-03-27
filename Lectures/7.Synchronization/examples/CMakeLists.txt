project(lecture7 CXX)
set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

add_executable(1_atomic 1_atomic.cpp common.cpp)
add_executable(2_mutex_and_spinlock 2_mutex_and_spinlock.cpp common.cpp)
add_executable(3_cond 3_cond.cpp common.cpp)
add_executable(4_barrier 4_barrier.cpp common.cpp)


