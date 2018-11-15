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

using error_t = std::variant<
	unclosed_comment,
	unexpected_char
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
