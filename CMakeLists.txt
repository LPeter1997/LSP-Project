cmake_minimum_required(VERSION 3.10)
project(Yoakke VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

add_subdirectory(compiler)
add_subdirectory(lsp_framework)
add_subdirectory(language_server)
