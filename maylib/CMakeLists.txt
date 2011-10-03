find_package (Threads)
add_library(maylib STATIC maylib.c err.c  heap.c  map.c  mem.c  str.c  utf.c syntree.c parser.c json.c stream.c tar.c mcl/mcl.c)
target_link_libraries (maylib ${CMAKE_THREAD_LIBS_INIT})
