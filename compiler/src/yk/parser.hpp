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
#include "ast.hpp"
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
		: m_Tokens(&toks), m_Index(0) {
	}

	/**
	 * Parses the global scope of the program, putting every node into a vector.
	 * @return A list of global AST nodes.
	 */
	std::vector<stmt*> decl_list();

	/**
	 * Parses a declaration.
	 * @return The parsed declaration node or nullptr if there was a problem.
	 */
	stmt* decl();

private:
	/**
	 * Peeks (but does not consume) forward in the token source.
	 * @param delta The amount to peek forward (1 by default).
	 * @return The peeked token.
	 */
	token const& peek(u32 delta = 0) const;

	/**
	 * Consumes the next token.
	 * @return The consumed token.
	 */
	token const& consume();

	/**
	 * Checks if we have reached the end of the token input. Note, that the
	 * input is required to have an EOF token at the end, meaning that this
	 * function returns true, when index >= tokens.size() - 1.
	 * @return True, if end of token input.
	 */
	bool is_eof() const;

	std::vector<token> const* m_Tokens;
	u32 m_Index;
};

} /* namespace yk */

#endif /* YK_PARSER_HPP */
