#include <iostream>
#include "lexer.hpp"

char const* test_src = R"(
hello {
	there everyone
}
)";

int main() {
	auto toks = yk::lexer::all(test_src);
	std::cerr << "Tokens: " << std::endl;
	for (auto const& t : toks) {
		std::cerr << "  [" << t.start().row() << " :: " << t.start().column() << " - " << t.end().column() << "] - '" << t.value() << "'" << std::endl;
	}
	auto s = yk::lexer::find_token_at(std::begin(toks), std::end(toks), yk::position::row_col(2, 10));
	std::cerr << std::endl << std::endl;
	if (s == std::end(toks)) {
		std::cerr << "No token there!" << std::endl;
	}
	else {
		std::cerr << "Token: " << yk::u32(s->type()) << " - '" << s->value() << "'" << std::endl;
	}
	return 0;
}
