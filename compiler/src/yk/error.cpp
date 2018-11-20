#include "error.hpp"

namespace yk {
namespace err {

static std::vector<error_t> error_list;

void init() {
	error_list = std::vector<error_t>();
}

void clear() {
	error_list.clear();
}

void report(error_t&& err) {
	error_list.push_back(std::move(err));
}

std::vector<error_t> const& errors() {
	return error_list;
}

} /* namespace err */
} /* namespace yk */
