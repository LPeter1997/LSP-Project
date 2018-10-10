#include <iostream>
#include "common.hpp"
#include "rpc.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include <fcntl.h>
#include <io.h>

void platform_init() {
	auto res = _setmode(_fileno(stdin), _O_BINARY);
	lsp_assert(res != -1);
}
#else
void platform_init() { }
#endif

void start() {
	platform_init();
	auto reader = lsp::msg_reader(std::cin);
	for (;;) {
		auto msg = reader.next();
		std::cerr << "Got message: " << msg.to_json().dump() << std::endl;
	}
}

int main() {
	start();
	return 0;
}
