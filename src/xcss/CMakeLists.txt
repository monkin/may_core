add_executable(xcss main.c parser.c)
add_dependencies(xcss maylib)
target_link_libraries(xcss maylib)
