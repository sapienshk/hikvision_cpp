cmake_minimum_required(VERSION 3.19)
project(test)
set(CMAKE_CXX_STANDARD 14)
include_directories(include)
link_directories("library")
add_executable(test main.cpp char_conversion.c)
add_library(jslib SHARED speaker_lib.cpp speaker_lib.h plate.cpp)
file(GLOB STATIC_LIBS "./library/*.lib")
target_link_libraries(test  ${STATIC_LIBS})
target_link_libraries(jslib  ${STATIC_LIBS})
