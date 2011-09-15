add_executable(test main.c)
add_dependencies(test maylib)
target_link_libraries(test maylib)
