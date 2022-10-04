cmake_minimum_required(VERSION 3.0)

project(ulsh)

add_executable(ulsh ulsh.c)

include_directories(/usr/include/readline)
target_link_libraries(ulsh readline)