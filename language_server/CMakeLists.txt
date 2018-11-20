set(ALL_SOURCES
	src/main.cpp
)

add_executable(yk_server ${ALL_SOURCES})
target_link_libraries(yk_server PRIVATE lsp_framework yk_lib)
