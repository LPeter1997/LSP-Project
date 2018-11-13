#include <iostream>
#include "common.hpp"
#include "lsp.hpp"
#include "lexer.hpp"

static lsp::position yk_to_lsp(yk::position const& p) {
	return lsp::position(p.row(), p.column());
}

static lsp::range yk_to_lsp(yk::range const& r) {
	return lsp::range(
		yk_to_lsp(r.start()),
		yk_to_lsp(r.end())
	);
}

static yk::position lsp_to_yk(lsp::position const& p) {
	return yk::position::row_col(p.line(), p.character());
}

static yk::range lsp_to_yk(lsp::range const& r) {
	return yk::range(
		lsp_to_yk(r.start()),
		lsp_to_yk(r.end())
	);
}

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
				.folding_range_provider(true)
			);
	}

	void on_text_document_opened(lsp::did_open_text_document_params const& p) override {
		m_Tokens = yk::lexer::all(p.text_document().text().c_str());
		std::cerr << "Tokens: " << std::endl;
		for (auto const& t : m_Tokens) {
			std::cerr << "  [" << t.start().row() << " :: " << t.start().column() << " - " << t.end().column() << "] - '" << t.value() << "'" << std::endl;
		}
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
		auto click_pos = lsp_to_yk(doc_pos);
		auto clicked_tok = yk::lexer::find_token_at(std::begin(m_Tokens), std::end(m_Tokens), click_pos);
		if (clicked_tok == std::end(m_Tokens)) {
			std::cerr << "Clicked on emptyness!" << std::endl;
			return {};
		}
		auto const& tok = *clicked_tok;
		std::cerr << "Clicked on: " << yk::u32(tok.type()) << " - '" << tok.value() << "'" << std::endl;
		return {
			lsp::document_highlight().highlight_range(yk_to_lsp(tok.range_()))
		};
	}

	std::vector<lsp::folding_range> on_folding_range(lsp::folding_range_params const& p) override {
		std::cerr << "Fold request!" << std::endl;
		std::vector<lsp::folding_range> result;
		for (auto const& t : m_Tokens) {
			if (t.type() == yk::token::NestedComment) {
				result.push_back(lsp::folding_range()
					.fold_range(yk_to_lsp(t.range_()))
				);
			}
		}
		return result;
	}

private:
	std::vector<yk::token> m_Tokens;
};

int main() {
	auto srvr = my_server();
	lsp::start_langserver(srvr, std::cin, std::cout);
	return 0;
}
