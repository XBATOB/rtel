# $Id: CMakeLists.txt 2212 2010-06-24 13:18:03Z xbatob $

include(CheckLibraryExists)
check_library_exists(readline readline "" HAVE_LIBREADLINE)

set(RTEL_SRCS
    main.c  node.c  rtel.c  tty.c
)

add_executable(rtel ${RTEL_SRCS})

if(HAVE_LIBREADLINE)
    set (RTEL_LIBS ${RTEL_LIBS} -lreadline)
endif(HAVE_LIBREADLINE)

target_link_libraries(rtel ${RTEL_LIBS})

install(TARGETS rtel DESTINATION bin)
