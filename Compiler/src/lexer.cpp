#include <cctype>
#include <cstring>
#include "lexer.hpp"

namespace yk {

std::vector<token> lexer::all(char const* src) {
	auto result = std::vector<token>();
	auto lex = lexer(src);
	while (true) {
		result.push_back(lex.next());
		if (result.back().type() == token::EndOfFile) {
			return result;
		}
	}
}

void lexer::advance(u32 n) {
	m_Source += n;
	m_Position.advance(n);
}

bool lexer::is_eof() const {
	return m_Source[0] == '\0';
}

bool lexer::parse_newline() {
	if (m_Source[0] == '\n') {
		// UNIX-style newline
		advance();
		return true;
	}
	else if (m_Source[0] == '\r') {
		if (m_Source[1] == '\n') {
			// Windows-style newline
			advance(2);
		}
		else {
			// OS-X 9-style newline
			advance();
		}
		return true;
	}
	else {
		return false;
	}
}

token lexer::make_simple(token::type_t ty, u32 len) {
	auto tok = token(m_Position, ty);
	advance(len);
	return tok;
}

token lexer::make_textual(token::type_t ty, u32 len) {
	auto tok = token(m_Position, ty, std::string(m_Source, len));
	advance(len);
	return tok;
}

/**
 * Checks if a character is suitable for an identifier. Basically needs to match
 * [A-Za-z0-9_].
 * @param ch The character to match.
 * @return True, if the character matches.
 */
static bool is_ident(char ch) {
	return std::isalnum(ch) || ch == '_';
}

token lexer::next() {
	while (true) {
		if (is_eof()) {
			return token(m_Position, token::EndOfFile);
		}
		if (parse_newline()) {
			continue;
		}
		if (std::isblank(m_Source[0])) {
			advance();
			continue;
		}

		switch (m_Source[0]) {
		case '(': return make_simple(token::LeftParen);
		case ')': return make_simple(token::RightParen);

		case '{': return make_simple(token::LeftBrace);
		case '}': return make_simple(token::RightBrace);

		case ':': return make_simple(token::Colon);
		}

		// Number
		if (std::isdigit(m_Source[0])) {
			u32 len = 1;
			while (std::isdigit(m_Source[len])) {
				++len;
			}
			return make_textual(token::Integer, len);
		}

		// Identifier
		if (is_ident(m_Source[0])) {
			u32 len = 1;
			while (is_ident(m_Source[len])) {
				++len;
			}

			if (std::strncmp(m_Source, "fn", len) == 0) {
				return make_simple(token::Keyword_Fn, len);
			}
			else {
				return make_textual(token::Identifier, len);
			}
		}

		// Unknown
		if (std::isgraph(m_Source[0])) {
			// Only error if a visible character
			yk_panic("Unknown token!");
			advance();
		}
		else {
			// Invisible, only skip in source
			++m_Source;
		}
	}

	yk_unreachable;
}

} /* namespace yk */
