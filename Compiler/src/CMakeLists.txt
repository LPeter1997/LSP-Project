cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(Compiler)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

set(ALL_SOURCES
	lexer.cpp
	main.cpp
)

add_executable(compiler ${ALL_SOURCES})
