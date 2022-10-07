cmake_minimum_required(VERSION 3.13)

project(ulsh)

add_subdirectory(builtins)

set(SOURCES 
    ulsh.c
    prompt.c
    parser.c
    exec.c
)

#add_compile_definitions(DEBUG_PARSER)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_executable(ulsh ${SOURCES})

include_directories(/usr/include/readline)
target_link_libraries(ulsh readline builtin)

add_subdirectory(tests)
enable_testing()

add_subdirectory(c0r3uti1s)
