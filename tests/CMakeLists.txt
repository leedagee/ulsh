project(tests)
enable_testing()

add_executable(test_builtin test_builtin.c ../jobs.c ../prompt.c ../exec.c)
target_link_libraries(test_builtin builtin readline)
add_test(NAME test_builtin COMMAND test_builtin)

add_executable(test_parser test_parser.c ../parser.c)
add_test(NAME test_parser COMMAND test_parser)

add_executable(test_parser_interactive test_parser_interactive.c ../parser.c)
target_link_libraries(test_parser_interactive readline)
