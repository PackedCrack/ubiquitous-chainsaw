//
// Created by qwerty on 2024-02-18.
//

#pragma once
#include "defines.hpp"
#include <utility>
#include <variant>
/// Concepts
namespace common
{
template<typename buffer_t>
concept const_buffer = requires(const std::remove_reference_t<buffer_t> buffer) {
    typename decltype(buffer)::value_type;
    typename decltype(buffer)::size_type;
    typename decltype(buffer)::const_pointer;
    typename decltype(buffer)::const_reference;
    typename decltype(buffer)::const_iterator;
    typename decltype(buffer)::const_reverse_iterator;


    requires sizeof(typename decltype(buffer)::value_type) == 1;

    { buffer.data() } -> std::same_as<typename decltype(buffer)::const_pointer>;
    { buffer.size() } -> std::same_as<typename decltype(buffer)::size_type>;
    { buffer.operator[](std::declval<typename decltype(buffer)::size_type>()) } -> std::same_as<typename decltype(buffer)::const_reference>;
    { buffer.cbegin() } -> std::same_as<typename decltype(buffer)::const_iterator>;
    { buffer.cend() } -> std::same_as<typename decltype(buffer)::const_iterator>;
    { buffer.crbegin() } -> std::same_as<typename decltype(buffer)::const_reverse_iterator>;
    { buffer.crend() } -> std::same_as<typename decltype(buffer)::const_reverse_iterator>;
};
template<typename buffer_t>
concept buffer = requires(std::remove_reference_t<buffer_t> buffer) {
    typename decltype(buffer)::value_type;
    typename decltype(buffer)::size_type;
    typename decltype(buffer)::pointer;
    typename decltype(buffer)::reference;
    typename decltype(buffer)::iterator;
    typename decltype(buffer)::reverse_iterator;

    requires sizeof(typename decltype(buffer)::value_type) == 1;

    { buffer.data() } -> std::same_as<typename decltype(buffer)::pointer>;
    { buffer.size() } -> std::same_as<typename decltype(buffer)::size_type>;
    { buffer.operator[](std::declval<typename decltype(buffer)::size_type>()) } -> std::same_as<typename decltype(buffer)::reference>;
    { buffer.begin() } -> std::same_as<typename decltype(buffer)::iterator>;
    { buffer.end() } -> std::same_as<typename decltype(buffer)::iterator>;
    { buffer.rbegin() } -> std::same_as<typename decltype(buffer)::reverse_iterator>;
    { buffer.rend() } -> std::same_as<typename decltype(buffer)::reverse_iterator>;
};
template<typename string_t>
concept string = requires(string_t str) {
    requires const_buffer<string_t>;
    { std::is_same_v<std::remove_cvref_t<string_t>, std::string> };
};
template<typename class_t, typename... ctor_args_t>
concept constructible_with = std::is_constructible_v<class_t, ctor_args_t...>;
template<typename queried_t, typename variant_t, size_t index = 0>
[[nodiscard]] consteval bool member_of_variant()
{
    if constexpr (index <= std::variant_size_v<variant_t>)
    {
        using variant_member_t = std::variant_alternative_t<index, variant_t>;
        if constexpr (std::same_as<queried_t, variant_member_t>)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}
}    // namespace common
/// Helpers
namespace common
{
template<typename small_t, typename large_t>
requires std::integral<std::remove_cvref_t<small_t>> && std::integral<std::remove_cvref_t<large_t>>
[[nodiscard]] constexpr small_t assert_down_cast(large_t&& large)
{
    static_assert((std::numeric_limits<small_t>::max)() <= (std::numeric_limits<std::remove_cvref_t<large_t>>::max)());
    ASSERT(large <= (std::numeric_limits<small_t>::max)(), "Overflow or wraparound!");
    return static_cast<small_t>(large);
};
template<typename enum_t>
requires std::is_enum_v<enum_t>
[[nodiscard]] constexpr bool enum_to_bool(enum_t&& properties)
{
#ifdef __cpp_lib_to_underlying
    return static_cast<bool>(std::to_underlying(std::forward<enum_t>(properties)));
#else
    return static_cast<bool>(static_cast<int64_t>(std::forward<enum_t>(properties)));
#endif
}
}    // namespace common
