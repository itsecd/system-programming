project(lecture6 CXX)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

set(CMAKE_CXX_STANDARD 20)

add_executable(1_create_thread 1_create_thread.cpp)
add_executable(2_cancel_thread 2_cancel_thread.cpp)
add_executable(3_1_thread_local 3_1_thread_local.cpp)
add_executable(3_2_tls 3_2_tls.cpp)
add_executable(5_nice_and_affinity 5_nice_and_affinity.cpp)
add_executable(4_signals 4_signals.cpp)
