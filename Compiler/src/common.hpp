/**
 * common.hpp
 *
 * @author Peter Lenkefi
 * @date 2018-11-12
 * @description Common type definitions, aliases and functionalities.
 */

#ifndef COMMON_HPP
#define COMMON_HPP

#include <cassert>
#include <cstdint>

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

} /* namespace yk */

#endif /* COMMON_HPP */
