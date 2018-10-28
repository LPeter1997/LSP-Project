#include <iostream>
#include "common.hpp"
#include "lsp.hpp"

struct my_server : public lsp::langserver {
	lsp::initialize_result on_initialize(lsp::initialize_params const& p) override {
		if (p.process_id()) {
			std::cerr << "Process id: " << *p.process_id() << std::endl;
		}
		if (auto const& wf = p.workspace_folders()) {
			for (auto const& w : *wf) {
				std::cerr << "Open workspace folder: " << w.name() << " - " << w.uri() << std::endl;
			}
		}
		std::cerr << std::flush;
		for(;;);
		return lsp::initialize_result();
	}
};

int main() {
	auto srvr = my_server();
	lsp::start_langserver(srvr, std::cin, std::cout);
	return 0;
}
