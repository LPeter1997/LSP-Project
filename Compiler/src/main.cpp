#include <iostream>
#include "error.hpp"
#include "lexer.hpp"

char const* test_src = R"(
	/*
hello {
	there everyone
}
)";

int main() {
	yk::err::init();
	auto toks = yk::lexer::all(test_src);
	auto const& errs = yk::err::errors();
	for (auto const& err : errs) {
		std::cerr << "Unclosed comment!" << std::endl;
	}
	return 0;
}
