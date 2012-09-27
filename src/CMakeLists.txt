find_package (Threads REQUIRED)
set(HEADERS
	lib.h err.h  heap.h  map.h  mem.h  str.h
	utf.h syntree.h parser.h json.h stream.h
	tar.h event.h floader.h mutex.h test.h
	arr.h)
add_library(may_core STATIC
	lib.c err.c  heap.c  map.c  mem.c  str.c
	utf.c syntree.c parser.c json.c stream.c
	tar.c event.c floader.c mutex.c test.c
	arr.c)
target_link_libraries(may_core ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS may_core DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include/may/core)
