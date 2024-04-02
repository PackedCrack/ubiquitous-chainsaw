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


namespace ble
{
template<typename descriptor_t, typename... make_args_t>
concept Descriptor = requires(const descriptor_t constDescriptor)
{
    awaitable_make<descriptor_t, make_args_t...>;
    { constDescriptor.uuid_as_str() } -> std::convertible_to<std::string>;
    { constDescriptor.protection_level() } -> std::convertible_to<ProtectionLevel>;
};

template<typename descriptor_t, typename... ctor_args_t>
requires Descriptor<descriptor_t, ctor_args_t ...>
[[nodiscard]] typename descriptor_t::awaitable_t make_descriptor(ctor_args_t&&... args)
{
    return descriptor_t::make(std::forward<ctor_args_t>(args)...);
}
}   // namespace ble