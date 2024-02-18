//
// Created by qwerty on 2024-02-18.
//

#pragma once
#include "defines.hpp"


/// Concepts
namespace common
{
template<typename buffer_t>
concept Buffer = requires(buffer_t buffer)
{
    { buffer.size() } -> std::convertible_to<typename decltype(buffer)::size_type>;
    { buffer.data() } -> std::convertible_to<typename decltype(buffer)::pointer>;
};
}   // common


/// Helpers
namespace common
{
template<typename small_t, typename large_t>
requires std::integral<std::remove_cvref_t<small_t>> && std::integral<std::remove_cvref_t<large_t>>
[[nodiscard]] small_t assert_down_cast(large_t&& large)
{
    static_assert((std::numeric_limits<small_t>::max)() < (std::numeric_limits<std::remove_cvref_t<large_t>>::max)());
    ASSERT(large <= (std::numeric_limits<small_t>::max)(), "Overflow or wraparound!");
    return static_cast<small_t>(large);
};
}   // common


