cmake_minimum_required( VERSION 3.25 )
project( Alepha )

add_compile_options( -I . ; -std=c++20 )

set( VERBOSE 1 )
set( CMAKE_VERBOSE_MAKEFILE true )

file( CREATE_LINK ${CMAKE_SOURCE_DIR} Alepha SYMBOLIC )

add_library( alepha SHARED Console.cpp ProgramOptions.cpp string_algorithms.cpp word_wrap.cpp )
link_libraries( alepha )

add_executable( example example.cc )
