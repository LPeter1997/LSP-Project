#include <cctype>
#include <cstring>
#include "lexer.hpp"

namespace yk {

bool operator==(position const& a, position const& b) {
	return a.row() == b.row() && a.column() == b.column();
}

bool operator!=(position const& a, position const& b) {
	return !(a == b);
}

bool operator<(position const& a, position const& b) {
	return (a.row() < b.row())
		|| ((a.row() == b.row()) && (a.column() < b.column()));
}

bool operator<=(position const& a, position const& b) {
	return (a == b) || (a < b);
}

bool operator>(position const& a, position const& b) {
	return !(a <= b);
}

bool operator>=(position const& a, position const& b) {
	return !(a < b);
}

////////////////////////////////////////////////////////////////////////////////

bool operator==(range const& a, range const& b) {
	return (a.start() == b.start()) && (a.end() == b.end());
}

bool operator!=(range const& a, range const& b) {
	return !(a == b);
}

/**
 * Note: The four operations below are not real comparsions for ranges that can
 * include other ranges.
 */

bool operator<(range const& a, range const& b) {
	return a.start() < b.start();
}

bool operator<=(range const& a, range const& b) {
	return a.start() <= b.start();
}

bool operator>(range const& a, range const& b) {
	return a.start() > b.start();
}

bool operator>=(range const& a, range const& b) {
	return a.start() >= b.start();
}

////////////////////////////////////////////////////////////////////////////////

static u32 calculate_length(token::type_t ty, std::string const& val) {
	switch (ty) {
	case token::EndOfFile:
		return 0;

	case token::LeftParen:
	case token::RightParen:
	case token::LeftBrace:
	case token::RightBrace:
	case token::Colon:
		return 1;

	case token::Keyword_Fn:
		return 2;

	case token::Identifier:
	case token::Integer:
		return val.length();
	}

	yk_unreachable;
}

range
token::calculate_range(position const& pos, type_t ty, std::string const& val) {
	auto pos2 = pos;
	pos2.advance(calculate_length(ty, val));
	return range(pos, pos2);
}

////////////////////////////////////////////////////////////////////////////////

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
		++m_Source;
		m_Position.newline();
		return true;
	}
	else if (m_Source[0] == '\r') {
		if (m_Source[1] == '\n') {
			// Windows-style newline
			m_Source += 2;
		}
		else {
			// OS-X 9-style newline
			++m_Source;
		}
		m_Position.newline();
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
