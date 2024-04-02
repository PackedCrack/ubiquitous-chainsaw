//
// Created by qwerty on 2024-04-02.
//

#pragma once
#ifdef WIN32
    #include "windows/CCharacteristic.hpp"
#else
    #error Only windows is implemented atm
#endif
#include "ble_common.hpp"


namespace ble
{
template<typename characteristic_t, typename... make_args_t>
concept Characteristic = requires()
{
    awaitable<characteristic_t, make_args_t...>;
};

template<typename characteristic_t, typename... ctor_args_t>
requires Characteristic<characteristic_t, ctor_args_t ...>
[[nodiscard]] typename characteristic_t::awaitable_t make_characteristic(ctor_args_t&&... args)
{
    return characteristic_t::make(std::forward<ctor_args_t>(args)...);
}
}   // namespace ble