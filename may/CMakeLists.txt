find_package (Threads)
find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS})
add_library(maylib STATIC maylib.c
	lib/err.c  lib/heap.c  lib/map.c  lib/mem.c  lib/str.c lib/utf.c
		lib/syntree.c lib/parser.c lib/json.c lib/stream.c lib/tar.c
	cl/ex.c cl/mcl.c cl/program.c)
target_link_libraries(maylib ${CMAKE_THREAD_LIBS_INIT} ${OPENCL_LIBRARIES})
