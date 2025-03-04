project(lecture4)

add_executable(1_kill 1_kill.cpp)
add_executable(2_sigaction 2_sigaction.cpp)

add_executable(3_error_in_handler 3_error_in_handler.cpp)
target_compile_options(3_error_in_handler PUBLIC -m32 )
target_link_options(3_error_in_handler PUBLIC -m32)

add_executable(3_malloc_in_handler 3_malloc_in_handler.cpp)

add_executable(4_sigwaitinfo 4_sigwaitinfo.cpp)
add_executable(5_rtsignal 5_rtsignal.cpp)
add_executable(6_eintr 6_eintr.cpp)
add_executable(7_hwexceptions 7_hwexceptions.cpp)
add_executable(8_abort 8_abort.cpp)
add_executable(9_alarm 9_alarm.cpp)
