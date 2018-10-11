#include <iostream>
#include "common.hpp"
#include "lsp.hpp"
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

struct my_server : public lsp::i_server {
	void init(lsp::initialize_params const& p) override {
		std::cerr << "Initialize!" << std::endl;
		if (auto p1 = p.process_id()) {
			std::cerr << "PID: " << *p1 << std::endl;
		}
		if (auto r = p.root()) {
			std::cerr << "Root: " << *r << std::endl;
		}
	}
};

int main() {
	platform_init();
	auto reader = lsp::msg_reader(std::cin);
	auto srvr = my_server();
	auto handler = lsp::msg_handler(std::cout, srvr);
	for (;;) {
		auto msg = reader.next();
		handler.handle(msg);
	}
	return 0;
}
