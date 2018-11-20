/**
 * common.hpp
 *
 * @author Peter Lenkefi
 * @date 2018-10-10
 * @description Common type definitions/aliases.
 */

#ifndef LSP_COMMON_HPP
#define LSP_COMMON_HPP

#include <cassert>
#include <cstdint>
#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include "json.hpp"

#define lsp_assert(x) assert(x)
#define lsp_panic(str) assert(false && str)
#define lsp_unreachable lsp_panic("Unreachable code!")
#define lsp_unimplemented lsp_panic("Unimplemented feature!")

// Making functions overloaded
#define FWD(...) \
std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

#define lift(x) 					\
[](auto&&... args) 					\
noexcept(noexcept(x(FWD(args)...))) \
-> decltype(x(FWD(args)...)) { 		\
	return x(FWD(args)...); 		\
}

namespace lsp {

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using json = nlohmann::json;

namespace detail {

template <typename>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template <typename T>
inline constexpr bool is_optional_v = is_optional<T>::value;

} /* namespace detail */

// fmap for optional
template <typename T, typename Fn,
	typename = std::enable_if_t<detail::is_optional_v<std::decay_t<T>>>>
auto operator|(T&& opt, Fn fn) {
	using fn_return_type = std::invoke_result_t<Fn, typename std::decay_t<T>::value_type>;

	if constexpr (detail::is_optional_v<fn_return_type>) {
		if (opt) {
			return fn(*std::forward<T>(opt));
		}
		else {
			return std::optional<typename fn_return_type::value_type>();
		}
	}
	else {
		if (opt) {
			return std::make_optional(fn(*std::forward<T>(opt)));
		}
		else {
            return std::optional<fn_return_type>();
		}
	}
}

} /* namespace lsp */

#endif /* LSP_COMMON_HPP */
