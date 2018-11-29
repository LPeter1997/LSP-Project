/**
 * ast.hpp
 *
 * @author Peter Lenkefi
 * @date 2018-11-26
 * @description All of the AST node definitions.
 */

#ifndef YK_AST_HPP
#define YK_AST_HPP

#include <optional>
#include "common.hpp"
#include "lexer.hpp"

#define make_heap(base) 						\
template <typename... TFwd> 					\
static base* make_##base(TFwd&&... params) { 	\
	return make(std::forward<TFwd>(params)...);	\
}

#define make_e() make_heap(expr)
#define make_s() make_heap(stmt)

namespace yk {

struct expr;
struct stmt;

/**
 * An atomic/terminal value, that could be an identifier, a symbol, or any
 * terminal that could appear in the AST. It has an optional position, because
 * later the compiler could inject nodes, whic means that positional position
 * will be lost there.
 */
template <typename T>
struct terminal {
	T value;
	std::optional<range> pos;

	terminal(terminal&&) = default;

	/**
	 * Creates a terminal value.
	 * @param val The value itself (like the token lexeme or type).
	 * @oaram pos The position of the value (nullopt by default).
	 */
	template <typename TF>
	terminal(TF&& val, std::optional<range>&& pos = std::nullopt)
		: value(std::forward<TF>(val)), pos(pos) {
	}
};

template <typename T>
terminal<std::decay_t<T>>
make_terminal(T&& val, std::optional<range> pos = std::nullopt) {
	return terminal<std::decay_t<T>>(val, std::move(pos));
}

/**
 * ADT of expression nodes.
 */
struct expr {
	/**
	 * Block expression.
	 */
	struct block {

	};

	////////////////////////////////////////////////////////////////////////////

	// The statement data type
	using data_t = std::variant<
		block
	>;

	data_t node;

	expr(expr&&) = default;

private:
	template <typename T>
	expr(T&& val)
		: node(std::forward<T>(val)) {
	}
};

/**
 * ADT of statement nodes.
 */
struct stmt {
	/**
	 * Function declaration statement.
	 */
	struct fdecl {
		// XXX(LPeter1997): Support multiple linkage types (in fdef too), like
		// stdcall, fastcall, ...

		/**
		 * Creates a function declaration.
		 * @param name The name identifier token of the declaration.
		 * @return The constructed declaration node.
		 */
		static fdecl make(token const& name);
		make_s();

		fdecl(fdecl&&) = default;

	private:
		fdecl(terminal<std::string>&& name,
			std::optional<terminal<std::string>>&& extName);

		terminal<std::string> m_Name;
		std::optional<terminal<std::string>> m_ExternName;
	};

	/**
	 * Function definition statement.
	 */
	struct fdef {
		// XXX(LPeter1997): See fdecl TODO comment

		/**
		 * Creates a function definition.
		 * @param name The name identifier token of the definition.
		 * @param block The body of the function.
		 * @return The constructed definition node.
		 */
		static fdef make(token const& name, expr::block&& block);
		make_s();

		fdef(fdef&&) = default;

	private:
		fdef(terminal<std::string>&& name,
			std::optional<terminal<std::string>>&& expName,
			expr::block&& body);

		terminal<std::string> m_Name;
		std::optional<terminal<std::string>> m_ExportName;
		expr::block m_Body;
	};

	////////////////////////////////////////////////////////////////////////////

	// The statement data type
	using data_t = std::variant<
		fdecl, fdef
	>;

	data_t node;

	stmt(stmt&&) = default;

private:
	template <typename T>
	stmt(T&& val)
		: node(std::forward<T>(val)) {
	}
};

} /* namespace yk */

#undef make_heap
#undef make_e
#undef make_s

#endif /* YK_AST_HPP */
