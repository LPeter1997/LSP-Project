cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(LspServer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

set(ALL_SOURCES
	lsp.cpp
	main.cpp
	rpc.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/../../Compiler/src)

link_directories(${CMAKE_BINARY_DIR})
find_library(compiler_lib
	NAMES compiler_lib libcompiler_lib libcompiler compiler
)

add_executable(langserver_test ${ALL_SOURCES})
target_link_libraries(langserver_test compiler_lib)
