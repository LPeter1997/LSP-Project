/**
 * common.hpp
 *
 * @author Peter Lenkefi
 * @date 2018-10-10
 * @description Common type definitions/aliases.
 */

#ifndef LSP_COMMON_HPP
#define LSP_COMMON_HPP

#include <cstdint>
#include <cassert>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include "json.hpp"

#define lsp_assert(x) assert(x)
#define lsp_panic(str) assert(false && str)
#define lsp_unreachable lsp_panic("Unreachable code!")
#define lsp_unimplemented lsp_panic("Unimplemented feature!")

namespace lsp {

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using str = std::string;

template <typename T>
using vec = std::vector<T>;

using json = nlohmann::json;

template <typename T>
using opt = std::optional<T>;

constexpr auto none = std::nullopt;

template <typename... Ts>
using sum = std::variant<Ts...>;

}

#endif /* LSP_COMMON_HPP */
