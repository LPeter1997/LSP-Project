#include <iostream>
#include "common.hpp"
#include "lsp.hpp"
#include "lexer.hpp"

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
		lsp_assert(p.content_changes().size() == 1);
		auto const& change = p.content_changes().front();
		lsp_assert(change.full_content());
		std::string const& src = change.text();
		m_Tokens = yk::lexer::all(src.c_str());
		std::cerr << "Tokens: " << std::endl;
		for (auto const& t : m_Tokens) {
			std::cerr << "  [" << t.start().row() << " :: " << t.start().column() << " - " << t.end().column() << "] - '" << t.value() << "'" << std::endl;
		}
	}

	void on_text_document_saved(lsp::did_save_text_document_params const& p) override {
		std::cerr << "Saved " << p.text_document().uri() << '!' << std::endl;
	}

	std::vector<lsp::document_highlight> on_text_document_highlight(lsp::text_document_position_params const& p) override {
		auto const& doc_pos = p.document_position();
		auto click_pos = yk::position::row_col(doc_pos.line(), doc_pos.character());
		auto clicked_tok = yk::lexer::find_token_at(std::begin(m_Tokens), std::end(m_Tokens), click_pos);
		if (clicked_tok == std::end(m_Tokens)) {
			std::cerr << "Clicked on emptyness!" << std::endl;
			return {};
		}
		auto const& tok = *clicked_tok;
		std::cerr << "Clicked on: " << yk::u32(tok.type()) << " - '" << tok.value() << "'" << std::endl;
		if (tok.type() == yk::token::LeftBrace) {
			std::cerr << "Clicked a left-brace!" << std::endl;
		}
		return {};
		/*
		return {
			lsp::document_highlight()
				.highlight_range(lsp::range(lsp::position(0, 0), lsp::position(0, 1)))
		};
		*/
	}

private:
	std::vector<yk::token> m_Tokens;
};

int main() {
	auto srvr = my_server();
	lsp::start_langserver(srvr, std::cin, std::cout);
	return 0;
}
