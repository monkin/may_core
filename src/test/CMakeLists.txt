add_executable(test main.c)
add_dependencies(test maylib)
include_directories(${OPENCL_INCLUDE_DIRS})
target_link_libraries(test maylib ${CMAKE_THREAD_LIBS_INIT} ${OPENCL_LIBRARIES})
