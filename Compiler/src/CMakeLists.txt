cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(Compiler)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

set(LIB_SOURCES
	lexer.cpp
)

set(NON_LIB_SOURCES
	main.cpp
)

add_library(compiler_lib ${LIB_SOURCES})

add_executable(compiler ${NON_LIB_SOURCES})
target_link_libraries(compiler compiler_lib)
