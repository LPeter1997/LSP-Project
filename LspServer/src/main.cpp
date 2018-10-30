#include <iostream>
#include "common.hpp"
#include "lsp.hpp"

struct my_server : public lsp::langserver {
	lsp::initialize_result initialize(lsp::initialize_params const& p) override {
		if (p.process_id()) {
			std::cerr << "Process id: " << *p.process_id() << std::endl;
		}
		if (auto const& wf = p.workspace_folders()) {
			for (auto const& w : *wf) {
				std::cerr << "Open workspace folder: " << w.name() << " - " << w.uri() << std::endl;
			}
		}
		return lsp::initialize_result()
			.capabilities(lsp::server_capabilities()
				.text_document_sync(lsp::text_document_sync_kind::full)
			);
	}

	void on_text_document_opened(lsp::did_open_text_document_params const& p) override {
		std::cerr << "----- Content:" << std::endl << p.text_document().text() << std::endl;
	}
};

int main() {
	auto srvr = my_server();
	lsp::start_langserver(srvr, std::cin, std::cout);
	return 0;
}
