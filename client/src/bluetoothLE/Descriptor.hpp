//
// Created by qwerty on 2024-04-02.
//

#pragma once
#ifdef WIN32
    #include "windows/CDescriptor.hpp"
#else
    #error Only windows is implemented atm
#endif
#include "ble_common.hpp"
// clang-format off


// clang-format on
namespace ble
{
template<typename descriptor_t, typename... make_args_t>
concept descriptor = requires(const descriptor_t constDescriptor) {
    awaitable_make<descriptor_t, make_args_t...>;
    string_uuid<descriptor_t>;
    typename descriptor_t::awaitable_read_t;

    { constDescriptor.read_value() } -> std::same_as<typename descriptor_t::awaitable_read_t>;
    { constDescriptor.protection_level() } -> std::same_as<ProtectionLevel>;
};
template<typename descriptor_t, typename... ctor_args_t>
requires descriptor<descriptor_t, ctor_args_t...>
[[nodiscard]] typename descriptor_t::awaitable_make_t make_descriptor(ctor_args_t&&... args)
{
    return descriptor_t::make(std::forward<ctor_args_t>(args)...);
}
}    // namespace ble
