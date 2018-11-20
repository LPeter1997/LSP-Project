set(ALL_SOURCES
	src/lsp/common.hpp
	src/lsp/json.hpp
	src/lsp/jwrap.hpp
	src/lsp/lsp.hpp
	src/lsp/lsp.cpp
	src/lsp/rpc.hpp
	src/lsp/rpc.cpp
)

add_library(lsp_framework ${ALL_SOURCES})

target_include_directories(lsp_framework PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
