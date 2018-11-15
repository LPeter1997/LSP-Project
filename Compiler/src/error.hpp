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

private:
	position m_Position;
	u32 m_Depth;
};

using error_t = std::variant<
	unclosed_comment
>;

/**
 * Initializes the error interface for usage.
 */
void init();

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
