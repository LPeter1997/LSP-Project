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
		std::cerr << "Trace: " << (lsp::u32)p.trace() << std::endl;
		if (auto i = p.initialization_options()) {
			std::cerr << i->dump() << std::endl;
		}
		{
			auto c = p.capabilities();
			auto const& w = c.workspace();
			if (auto e = w.workspace_edit()) {
				std::cerr << "Doc change: " << e->document_changes() << std::endl;
			}
		}
		if (auto const& wf = p.workspace_folders()) {
			for (auto const& w : *wf) {
				std::cerr << "WS folder: " << w.name() << " - " << w.uri() << std::endl;
			}
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
		//std::cerr << "MSG: " << msg.to_json().dump() << std::endl;
		handler.handle(msg);
	}
	return 0;
}
