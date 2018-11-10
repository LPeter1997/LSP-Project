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
				.document_highlight_provider(true)
			);
	}

	void on_text_document_opened(lsp::did_open_text_document_params const& p) override {
		std::cerr << "Opened with content: " << p.text_document().text() << std::endl;
	}

	void on_text_document_changed(lsp::did_change_text_document_params const& p) override {
		std::cerr << "Change:" << std::endl;
		for (auto const& c : p.content_changes()) {
			std::cerr << " - " << c.text() << std::endl;
		}
	}

	void on_text_document_saved(lsp::did_save_text_document_params const& p) override {
		std::cerr << "Saved " << p.text_document().uri() << '!' << std::endl;
	}

	std::vector<lsp::document_highlight> on_text_document_highlight(lsp::text_document_position_params const& p) override {
		return {
			lsp::document_highlight()
				.highlight_range(lsp::range(lsp::position(0, 0), lsp::position(0, 1)))
		};
	}
};

int main() {
	auto srvr = my_server();
	lsp::start_langserver(srvr, std::cin, std::cout);
	return 0;
}
