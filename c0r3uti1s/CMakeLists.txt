add_executable(ls ls.c)
add_executable(echo echo.c)
add_executable(cat cat.c)
add_executable(kill kill.c)
add_executable(mkdir mkdir.c)
add_executable(touch touch.c)

add_executable(less less.c)
target_link_libraries(less ncurses)