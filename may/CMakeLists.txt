find_package(OpenCL REQUIRED)
find_package(DevIL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIRS} ${IL_INCLUDE_DIR})
add_library(maylib STATIC maylib.c
	lib/err.c  lib/heap.c  lib/map.c  lib/mem.c  lib/str.c lib/utf.c
	lib/syntree.c lib/parser.c lib/json.c lib/stream.c lib/tar.c lib/event.c
	cl/ex.c cl/mcl.c cl/program.c cl/error.c cl/image.c cl/filter.c
	cl/filters/const.c)
target_link_libraries(maylib ${OPENCL_LIBRARIES} ${IL_LIBRARIES})
