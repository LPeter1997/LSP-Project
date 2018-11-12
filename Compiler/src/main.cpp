#include <iostream>
#include "lexer.hpp"

int main() {
	auto toks = yk::lexer::all("(){}hello ther 123");
	for (auto const& tok : toks) {
		std::cout
			<< '[' << tok.pos().row() << ":" << tok.pos().column() << "]: "
			<< yk::u32(tok.type()) << " - '" << tok.value() << '\''
			<< std::endl;
	}
	return 0;
}
