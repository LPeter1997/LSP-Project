cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(LspServer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

set(ALL_SOURCES
	main.cpp
	rpc.cpp
)

add_executable(langserver_test ${ALL_SOURCES})
