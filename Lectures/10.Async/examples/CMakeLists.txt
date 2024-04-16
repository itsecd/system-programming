project(lecture10)
set(CMAKE_CXX_STANDARD 20)

add_executable(1_tcp_client 1_tcp_client.cpp common.cpp)
add_executable(1_tcp_server_with_poll 1_tcp_server_with_poll.cpp common.cpp)
add_executable(2_aio_performance 2_aio_performance.cpp common.cpp)
add_executable(3_aio_notifications 3_aio_notifications.cpp)
add_executable(4_unix_file_client 4_unix_file_client.cpp)
add_executable(4_unix_file_server 4_unix_file_server.cpp)