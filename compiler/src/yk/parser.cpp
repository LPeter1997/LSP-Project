#include "ast.hpp"
#include "error.hpp"
#include "parser.hpp"

namespace yk {

std::vector<stmt*> parser::all(std::vector<token> const& toks) {
	auto p = parser(toks);
	return p.decl_list();
}

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

		auto* fn_name = expect(token::Identifier, "function name");
		if (!fn_name) return nullptr;

		// 'fn' name

		expect(token::LeftParen, "'('");
		expect(token::RightParen, "')'");

		// 'fn' name()

		// If foreign token, it's a declaration. Definition otherwise.

		if (auto* foreign_kw = match(token::Keyword_Foreign)) {
			// Declaration
			expect(token::Semicolon, "';'");
			return stmt::fdecl::make_stmt(*fn_name);
		}
		else {
			auto body = block();
			if (!body) return nullptr;

			// XXX(LPeter1997): If there was no body, we could return a
			// pseudo-body so the function signature would be registered for
			// semantic checking.

			return stmt::fdef::make_stmt(*fn_name, std::move(*body));
		}
	}
	else {
		err::report(err::unexpected_token(peek(), "declaration"));
		// Consume so we don't get stuck here
		// XXX(LPeter1997): Might be a better strategy to sync to the next decl
		// token
		consume();
	}
	return nullptr;
}

std::optional<expr::block> parser::block() {
	if (auto* lbrace = expect(token::LeftBrace, "'{'")) {
		auto* rbrace = expect(token::RightBrace, "'}'");
		// XXX(LPeter1997): Auto-close to continue? (no need to throw away the
		// block)
		if (!rbrace) return std::nullopt;

		return expr::block::make(*lbrace, *rbrace, {}, nullptr);
	}
	return std::nullopt;
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

token const* parser::expect(token::type_t tag, char const* desc) {
	token const& t = peek();
	if (t.type() == tag) {
		return &consume();
	}
	else {
		if (!is_eof()) {
			consume();
		}
		err::report(err::expected_token(t, desc));
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
