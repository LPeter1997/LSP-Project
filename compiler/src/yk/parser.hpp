/**
 * parser.hpp
 *
 * @author Peter Lenkefi
 * @date 2018-11-26
 * @description This is the parser module that parses the token stream into an
 * AST representation.
 */

#ifndef YK_PARSER_HPP
#define YK_PARSER_HPP

#include <vector>
#include "lexer.hpp"

namespace yk {

/**
 * The parser object itself. Takes a list of tokens and constructs an AST by the
 * language grammar.
 */
struct parser {
	/**
	 * Creates a parser for a given token source.
	 * @param toks The tokens to parse from.
	 */
	explicit parser(std::vector<token> const& toks)
		: m_Tokens(&toks) {
	}

private:
	std::vector<token> const* m_Tokens;
};

} /* namespace yk */

#endif /* YK_PARSER_HPP */
