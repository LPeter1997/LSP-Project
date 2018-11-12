/**
 * lexer.hpp
 *
 * @author Peter Lenkefi
 * @date 2018-11-12
 * @description The whole lexical analysis and the needed structures.
 */

#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include "common.hpp"

namespace yk {

/**
 * Holds positional information with a row and column number.
 */
struct position {
	/**
	 * Creates a position instance using the named constructor idiom.
	 * @param r The row of the position. (0 based)
	 * @param c The column of the position. (0 based)
	 * @return The newly created position object.
	 */
	static position row_col(u32 r, u32 c) {
		return position(r, c);
	}

	/**
	 * Returns a new, empty position pointing at 0, 0.
	 * @return A position with row = 0, column = 0.
	 */
	static position zero() {
		return row_col(0, 0);
	}

	auto const& row() const { return m_Row; }
	auto const& column() const { return m_Col; }

	/**
	 * Advances the position in the column axis.
	 * @param n The amount to advance. 1 by default.
	 * @return A reference to this.
	 */
	position& advance(u32 n = 1) {
		m_Col += n;
		return *this;
	}

	/**
	 * Advances the position to the new line and first column.
	 * @return A reference to this.
	 */
	position& newline() {
		++m_Row;
		m_Col = 0;
		return *this;
	}

private:
	explicit position(i32 r, i32 c)
		: m_Row(r), m_Col(c) {
	}

	u32 m_Row;	// Row (y) number, 0 based.
	u32 m_Col; // Column (x) number, 0 based.
};

/**
 * The atomic result of the lexical analysis.
 */
struct token {
	/**
	 * The normalized token types that the parser can work with.
	 */
	enum type_t {
		EndOfFile,	// EOF
		LeftParen,	// '('
		RightParen,	// ')'
		LeftBrace,	// '{'
		RightBrace,	// '}'
		Colon,		// ':'
		Keyword_Fn,	// 'fn'
		Identifier,	// [A-Za-z_][A-Za-z0-9_]*
		Integer,	// [0-9]+
	};

	token(token const&) = default;
	token(token&&) = default;
	token& operator=(token const&) = default;
	token& operator=(token&&) = default;

	/**
	 * Creates a token.
	 * @param pos The starting position of the token.
	 * @param ty The type of the token.
	 * @param val The textual value of the token. Only required if it contains
	 * semantic information, like numbers and identifiers. Empty by default.
	 */
	explicit token(position const& pos, type_t ty, std::string&& val = "")
		: m_Position(pos), m_Type(ty), m_Value(std::move(val)) {
	}

	position const& pos() const { return m_Position; }
	type_t const& type() const { return m_Type; }
	std::string const& value() const { return m_Value; }

private:
	position m_Position;
	type_t m_Type;
	std::string m_Value;
};

/**
 * The lexer object that parses a string into tokens.
 */
struct lexer {
	/**
	 * A utility function that lexer a whole source string until the end and
	 * returns the resulting tokens in a vector (including the EndOfFile token).
	 * @return A vector of tokens.
	 */
	static std::vector<token> all(char const* src);

	/**
	 * Creates a lexer for a given source text.
	 * @param src The source as a null-terminated character sequence.
	 */
	explicit lexer(char const* src)
		: m_Source(src), m_Position(position::zero()) {
	}

	/**
	 * Returns the next token from the source file.
	 * @return The next token. The type will be EndOfFile, if the end of source
	 * (the null terminator) is reached.
	 */
	token next();

private:
	/**
	 * Advances the source and the horizontal position (columns) by a given
	 * amount.
	 * @param n The amount to advance (1 by default).
	 */
	void advance(u32 n = 1);

	/**
	 * Check if the end of source is reached.
	 * @return True, if the end of source (null-terminator) is reached.
	 */
	bool is_eof() const;

	/**
	 * Tries to parse a newline from the source. If a newline is found, skips
	 * it in the source.
	 * @return True, if a newline was found and skipped.
	 */
	bool parse_newline();

	/**
	 * Creates a token at the current position and advances the horizontal
	 * position (columns) by a given amount.
	 * @param ty The type of the token to create.
	 * @param len The horizontal length of the token (amount to advance). 1 by
	 * default.
	 * @return A new token at the current position with the given type.
	 */
	token make_simple(token::type_t ty, u32 len = 1);

	/**
	 * Creates a token at the current position and advances the horizontal
	 * position (columns) by a given amount. Also provides the sliced text for
	 * the token.
	 * @param ty The type of the token to create.
	 * @param len The horizontal length of the token (amount to advance).
	 * @return A new token at the current position with the given type with the
	 * text from the current position (to the given length).
	 */
	token make_textual(token::type_t ty, u32 len);

	char const* m_Source; // Source pointer
	position m_Position; // Current position
};

} /* namespace yk */

#endif /* LEXER_HPP */
