/**
 * common.hpp
 *
 * @author Peter Lenkefi
 * @date 2018-11-12
 * @description Common type definitions, aliases and functionalities.
 */

#ifndef YK_COMMON_HPP
#define YK_COMMON_HPP

#include <cassert>
#include <cstdint>
#include <variant>

#define yk_assert(x) assert(x)
#define yk_panic(str) assert(false && str)
#define yk_unreachable yk_panic("Unreachable code!")
#define yk_unimplemented yk_panic("Unimplemented feature!")

namespace yk {

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

// A simple mechanism to make variant visitation a bit easier with lambdas.

template <typename... Ts>
struct overload_set : Ts... {
    using Ts::operator()...;

    template <typename... Ps>
    constexpr overload_set(Ps&&... params)
        : Ts(std::forward<Ps>(params))... {
    }
};

template <typename... Ts>
constexpr auto overload(Ts&&... fns) {
    return overload_set<std::decay_t<Ts>...>(std::forward<Ts>(fns)...);
}

template <typename... Vs>
constexpr auto match(Vs&&... vs) {
    return [&vs...](auto&&... fs) -> decltype(auto) {
        auto visitor = overload(std::forward<decltype(fs)>(fs)...);
        return std::visit(visitor, std::forward<Vs>(vs)...);
    };
}

} /* namespace yk */

#endif /* YK_COMMON_HPP */
