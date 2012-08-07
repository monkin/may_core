add_executable(run main.c)
add_dependencies(run may_core)
target_link_libraries(run may_core)
