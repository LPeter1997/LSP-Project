#include "ast.hpp"
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

		auto* fn_name = match(token::Identifier);
		// XXX(LPeter1997): Expect & error instead of assert
		yk_assert(fn_name);

		// 'fn' name

		auto* lparen = match(token::LeftParen);
		yk_assert(lparen);
		auto* rparen = match(token::RightParen);
		yk_assert(rparen);

		// 'fn' name()

		// If foreign token, it's a declaration. Definition otherwise.

		if (auto* foreign_kw = match(token::Keyword_Foreign)) {
			// Declaration
			auto* semicol = match(token::Semicolon);
			yk_assert(semicol);

			return stmt::fdecl::make_stmt(*fn_name);
		}
		else {
			auto* body = block();
			yk_assert(body);

			// XXX(LPeter1997): If there was no body, we could return a
			// pseudo-body so the function signature would be registered for
			// semantic checking.

			// XXX(LPeter1997): Body is heap-allocated, free that?

			return stmt::fdef::make_stmt(*fn_name, std::move(*body));
		}
	}
	return nullptr;
}

expr::block* parser::block() {
	if (auto* lbrace = match(token::LeftBrace)) {
		auto* rbrace = match(token::RightBrace);
		yk_assert(rbrace);

		// XXX(LPeter1997): The API is not correct for now...
		return expr::block::make(*lbrace, *rbrace, {}, nullptr);
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
