cmake_minimum_required(VERSION 3.6.0)

#project(libswt)
set(CMAKE_MACOSX_RPATH 1)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
include_directories(./src)
if (DEBUG)
    add_definitions(-O0 -ggdb -fsanitize=address -fno-omit-frame-pointer)
    link_directories(/Library/Developer/CommandLineTools/usr/lib/clang/9.0.0/lib/darwin/)
else()
    add_definitions(-O3 -fvisibility=hidden)
endif()

file(GLOB_RECURSE SOURCE ./src/*.c)
#add_library(swt STATIC ${SOURCE})
add_library(swt SHARED ${SOURCE})
target_link_libraries(swt
                      android
                      jnigraphics
                      log
                      m)