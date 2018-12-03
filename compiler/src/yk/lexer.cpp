#include <cctype>
#include <cstring>
#include "error.hpp"
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

	case token::Keyword_Foreign:
		return 7;

	case token::Identifier:
	case token::Integer:
		return val.length();

	case token::LineComment:
	case token::NestedComment:
		yk_panic("A comment should not be asked about it's length!");
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
		case '/': {
			if (m_Source[1] == '/') {
				// Line comment
				// We don't actually need to keep track of positioning, that's
				// why we modify the source directly
				auto beg_pos = m_Position;
				advance(2);
				auto end_pos = m_Position;
				while (true) {
					if (is_eof()) {
						break;
					}
					else if (parse_newline()) {
						break;
					}
					else if (std::isgraph(m_Source[0])
						|| std::isblank(m_Source[0])) {
						// Visual
						advance(1);
						end_pos = m_Position;
					}
					else {
						// Non-visual
						++m_Source;
					}
				}
				return token(range(beg_pos, end_pos), token::LineComment);
			}
			else if (m_Source[1] == '*') {
				// Nested comment
				// Positioning is important here, as nested comments can end in
				// the middle of the line, where more code follows. For the
				// actual code we need those token positions correctly.
				auto beg_pos = m_Position;
				advance(2);

				// We will use a counter instead of a stack, using a stack to
				// keep track of nesting would be a huge overkill.
				u32 depth = 1;
				while (depth > 0) {
					if (is_eof()) {
						// XXX(LPeter1997): Better positioning? (end of last
						// visible)
						err::report(err::unclosed_comment(
							m_Position, depth
						));
						break;
					}
					else if (parse_newline()) {
						// Do nothing
					}
					else if (m_Source[0] == '/' && m_Source[1] == '*') {
						// Nest
						advance(2);
						++depth;
					}
					else if (m_Source[0] == '*' && m_Source[1] == '/') {
						// Un-nest
						advance(2);
						--depth;
					}
					else if (std::isgraph(m_Source[0])
						|| std::isblank(m_Source[0])) {
						// Occupies space, advance in position
						advance(1);
					}
					else {
						// Some control character
						++m_Source;
					}
				}
				return token(range(beg_pos, m_Position), token::NestedComment);
			}
		} break;

		case ';': return make_simple(token::Semicolon);

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
			else if (std::strncmp(m_Source, "foreign", len) == 0) {
				return make_simple(token::Keyword_Foreign, len);
			}
			else {
				return make_textual(token::Identifier, len);
			}
		}

		// Unknown
		if (std::isgraph(m_Source[0])) {
			// Only error if a visible character
			err::report(err::unexpected_char(m_Position, m_Source[0]));
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
