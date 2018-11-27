#include "ast.hpp"

namespace yk {

stmt::fdecl stmt::fdecl::make(token const& name) {
	return fdecl(make_terminal(name.value(), name.range_()), std::nullopt);
}

stmt::fdecl::fdecl(terminal<std::string>&& name,
	std::optional<terminal<std::string>>&& extName)
	: m_Name(std::move(name)), m_ExternName(std::move(extName)) {
}

} /* namespace yk */

