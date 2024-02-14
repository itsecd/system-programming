project(lecture2)

set(CMAKE_CXX_STANDARD 17)

add_executable(1_copy_file "1_copy_file.cpp")
add_executable(2_dup  "2_dup.cpp")
add_executable(3_lseek  "3_lseek_and_stat.cpp")
add_executable(4_fopen  "4_fopen.cpp")
add_executable(5_flock  "5_flock.cpp")
add_executable(6_ioctl  "6_ioctl.cpp")
add_executable(7_inotify "7_inotify.cpp")