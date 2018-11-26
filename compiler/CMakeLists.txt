set(LIB_SOURCES
	src/yk/ast.hpp
	src/yk/common.hpp
	src/yk/error.hpp
	src/yk/error.cpp
	src/yk/lexer.hpp
	src/yk/lexer.cpp
	src/yk/parser.hpp
)

set(CLI_SOURCES
	src/yk/main.cpp
)

add_library(yk_lib ${LIB_SOURCES})

target_include_directories(yk_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_executable(yk ${CLI_SOURCES})
target_link_libraries(yk PRIVATE yk_lib)
