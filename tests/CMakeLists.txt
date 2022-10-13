project(tests)
enable_testing()

add_executable(test_builtin test_builtin.c ../jobs.c ../prompt.c ../exec.c)
target_link_libraries(test_builtin builtin readline)
add_test(NAME test_builtin COMMAND test_builtin)
