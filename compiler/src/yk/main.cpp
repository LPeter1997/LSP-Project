#include <iostream>
#include "error.hpp"
#include "lexer.hpp"
#include "parser.hpp"

// XXX(LPeter1997): We could remove std::vector dependency everywhere by using
// iterator pairs. That is more idiomatic C++.

// XXX(LPeter1997): Rename error module to diagnostics

// XXX(LPeter1997): End the weird namings!
// Sometimes we use shorthands like pos(), or use postfixes like range_().
// End it once and for all!

char const* test_src = R"(

	fn main() foreign;
	fn foo() foreign;

)";

int main() {
	yk::err::init();
	auto toks = yk::lexer::all(test_src);
	auto decls = yk::parser::all(toks);
	std::cout << decls.size() << " no. declarations" << std::endl;
	return 0;
}
