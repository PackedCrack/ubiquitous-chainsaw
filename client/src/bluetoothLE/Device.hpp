//
// Created by qwerty on 2024-02-14.
//
#pragma once
#ifdef WIN32
    #include "windows/CDevice.hpp"
#else
    #error Only windows is implemented atm
#endif


namespace ble
{
template<typename device_t, typename... make_args_t>
concept Device = requires()
{
    awaitable<device_t, make_args_t...>;
};

template<typename device_t, typename... ctor_args_t>
requires Device<device_t, ctor_args_t...>
[[nodiscard]] typename device_t::awaitable_t make_device(ctor_args_t&&... args)
{
    return device_t::make(std::forward<ctor_args_t>(args)...);
}
}   // namespace ble