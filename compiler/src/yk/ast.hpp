/**
 * ast.hpp
 *
 * @author Peter Lenkefi
 * @date 2018-11-26
 * @description All of the AST node definitions.
 */

#ifndef YK_AST_HPP
#define YK_AST_HPP

#include "common.hpp"
#include "lexer.hpp"

namespace yk {

struct expr;
struct stmt;

/**
 * ADT of statement nodes.
 */
struct stmt {
	/**
	 * Function declaration statement.
	 */
	struct fdecl {

	};

	/**
	 * Function definition statement.
	 */
	struct fdef {

	};

	// The statement data type
	using data_t = std::variant<
		fdecl, fdef
	>;
};

/**
 * ADT of expression nodes.
 */
struct expr {

};

} /* namespace yk */

#endif /* YK_AST_HPP */
