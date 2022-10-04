cmake_minimum_required(VERSION 3.0)

project(ulsh)

set(SOURCES 
    ulsh.c
    prompt.c
)

add_executable(ulsh ${SOURCES})

include_directories(/usr/include/readline)
target_link_libraries(ulsh readline)