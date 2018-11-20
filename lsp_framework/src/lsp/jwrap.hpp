/**
 * jwrap.hpp
 *
 * @author Peter Lenkefi
 * @date 2018-10-27
 * @description This is a wrapper structure for nlohmann's JSON library to make
 * serialization and de-serialization easier easier.
 */

#ifndef LSP_JWRAP_HPP
#define LSP_JWRAP_HPP

#include "common.hpp"

namespace lsp {

struct jwrap {
	jwrap(json const& js)
		: m_JSON(js) {
	}

	template <typename T = json>
	T get(char const* name) const {
		lsp_assert(m_JSON.find(name) != m_JSON.end());
		return m_JSON[name].get<T>();
	}

	template <typename T = json>
	std::optional<T> opt(char const* name) const {
		if (auto it = m_JSON.find(name); it != m_JSON.end()) {
			return it->get<T>();
		}
		else {
			return std::nullopt;
		}
	}

private:
	json const& m_JSON;
};

struct jbuild {
	explicit jbuild() = default;

	template <typename T>
	auto& set(char const* name, T&& val) {
		m_JSON[name] = std::forward<T>(val);
		return *this;
	}

	template <typename T>
	auto& opt(char const* name, T&& val) {
		if (val) {
			m_JSON[name] = *std::forward<T>(val);
		}
		return *this;
	}

	template <typename T>
	auto& opt_as_null(char const* name, T&& val) {
		if (val) {
			m_JSON[name] = *std::forward<T>(val);
		}
		else {
			m_JSON[name] = nullptr;
		}
		return *this;
	}

	json get() {
		return std::move(m_JSON);
	}

private:
	json m_JSON;
};

} /* namespace lsp */

#endif /* LSP_JWRAP_HPP */
