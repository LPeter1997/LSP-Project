#include "parser.hpp"

namespace yk {

std::vector<stmt*> parser::decl_list() {
	std::vector<stmt*> result;
	while (!is_eof()) {
		if (auto* d = decl()) {
			result.push_back(d);
		}
	}
	return result;
}

stmt* parser::decl() {
	if (auto* fn_kw = match(token::Keyword_Fn)) {
		// 'fn'
		// XXX(LPeter1997): identifier '(' ')' ...
	}
	return nullptr;
}

// Helper functionality

token const* parser::match(token::type_t tag) {
	token const& t = peek();
	if (t.type() == tag) {
		return &consume();
	}
	else {
		return nullptr;
	}
}

token const& parser::peek(u32 delta) const {
	return (*m_Tokens)[m_Index + delta];
}

token const& parser::consume() {
	auto const& t = peek();
	++m_Index;
	return t;
}

bool parser::is_eof() const {
	return m_Index >= m_Tokens->size() - 1;
}

} /* namespace yk */
