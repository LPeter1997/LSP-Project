/**
 * error.hpp
 *
 * @author Peter Lenkefi
 * @date 2018-11-15
 * @description The error reporting and retrieving interface for the entire
 * compilation process.
 */

#ifndef YK_ERROR_HPP
#define YK_ERROR_HPP

#include <variant>
#include <vector>
#include "common.hpp"
#include "lexer.hpp"

namespace yk {
namespace err {

/**
 * Error when a multi-line comment is not terminated properly at the end of
 * file.
 */
struct unclosed_comment {
	/**
	 * Creates an unclosed comment error.
	 * @param pos The end of file position (actually it's better to use the
	 * position pointing after the last token).
	 * @param d The depth that the comment was nested (how many comment close
	 * symbols are missing).
	 */
	explicit unclosed_comment(position const& pos, u32 d)
		: m_Position(pos), m_Depth(d) {
	}

	position const& pos() const { return m_Position; }
	u32 depth() const { return m_Depth; }

	range err_range() const { return range(pos(), 1); }

private:
	position m_Position;
	u32 m_Depth;
};

/**
 * Error when we encounter an unknown/unexpected character in the source code.
 */
struct unexpected_char {
	/**
	 * Creates an unexpected character error.
	 * @param pos The position of the character.
	 * @param ch The character itself.
	 */
	explicit unexpected_char(position const& pos, char ch)
		: m_Position(pos), m_Char(ch) {
	}

	position const& pos() const { return m_Position; }
	char character() const { return m_Char; }

	u32 character_code() const { return static_cast<u32>(character()); }
	range err_range() const { return range(pos(), 1); }

private:
	position m_Position;
	char m_Char;
};

/**
 * Error when we encounter something at parsing that we cannot continue from.
 * Like there is a token that cannot possibly start a declaration when we need
 * to parse one.
 */
struct unexpected_token {
	/**
	 * Creates an unexpected token error.
	 * @param tok The token that came unexpected for the parser.
	 * @param instead The description that was expected instead (nullptr by
	 * default).
	 */
	explicit unexpected_token(token const& tok, char const* instead = nullptr)
		: m_Token(tok), m_Instead(instead) {
	}

	token const& tok() const { return m_Token; }
	char const* expected_instead() const { return m_Instead; }

	range err_range() const { return tok().range_(); }

private:
	token m_Token;
	char const* m_Instead;
};

/**
 * Error when we expect a kind of token but get something else.
 */
struct expected_token {
	/**
	 * Creates an expected token error.
	 * @param tok The token that came instead of the expected token.
	 * @param expectation The description that was expected.
	 */
	explicit expected_token(token const& tok, char const* expectation)
		: m_Got(tok), m_Expectation(expectation) {
	}

	token const& got() const { return m_Got; }
	char const* expectation() const { return m_Expectation; }

	range err_range() const { return got().range_(); }

private:
	token m_Got;
	char const* m_Expectation;
};

using error_t = std::variant<
	unclosed_comment,
	unexpected_char,
	unexpected_token,
	expected_token
>;

/**
 * Initializes the error interface for usage.
 */
void init();

/**
 * Clears the error list.
 */
void clear();

/**
 * Reports an error.
 * @param err The error to report.
 */
void report(error_t&& err);

/**
 * Retrieves all the errors so far.
 * @return The list of reported errors.
 */
std::vector<error_t> const& errors();

} /* namespace err */
} /* namespace yk */

#endif /* YK_ERROR_HPP */
