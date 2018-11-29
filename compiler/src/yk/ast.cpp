#include "ast.hpp"

namespace yk {

// Function declaration

stmt::fdecl stmt::fdecl::make(token const& name) {
	return fdecl(make_terminal(name.value(), name.range_()), std::nullopt);
}

stmt::fdecl::fdecl(terminal<std::string>&& name,
	std::optional<terminal<std::string>>&& extName)
	: m_Name(std::move(name)), m_ExternName(std::move(extName)) {
}

// Function definition

stmt::fdef stmt::fdef::make(token const& name, expr::block&& block) {
	return fdef(
		make_terminal(name.value(), name.range_()),
		std::nullopt,
		std::move(block)
	);
}

stmt::fdef::fdef(terminal<std::string>&& name,
	std::optional<terminal<std::string>>&& expName,
	expr::block&& body)
	: m_Name(std::move(name)), m_ExportName(std::move(expName)),
	m_Body(std::move(body)) {
}

} /* namespace yk */

