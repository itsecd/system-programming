project(lecture5)

set(CMAKE_CXX_STANDARD 17)

add_executable(file 1_file.cpp)
add_executable(mmap 3_mmap.cpp)
add_executable(shm 4_shm.cpp)
add_executable(pipe 6_pipe.cpp)
add_executable(named_pipe 7_named_pipe.cpp)
add_executable(semaphore 5_semaphore.cpp)
add_executable(mqueue 9_mqueue.cpp)
add_executable(pipe_buffer 8_pipe_buffer.cpp)
add_executable(guard_page 2_guard_page.cpp)
