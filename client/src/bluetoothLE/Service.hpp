//
// Created by qwerty on 2024-04-02.
//

#pragma once
#ifdef WIN32
    #include "windows/CService.hpp"
#else
    #error Only windows is implemented atm
#endif
#include "ble_common.hpp"


namespace ble
{
template<typename service_t, typename... make_args_t>
concept Service = requires(uint64_t address)
{
    awaitable<service_t, make_args_t...>;
};

template<typename service_t, typename... ctor_args_t>
requires Service<service_t, ctor_args_t ...>
[[nodiscard]] typename service_t::awaitable_t make_service(ctor_args_t&&... args)
{
    return service_t::make(std::forward<ctor_args_t>(args)...);
}
}   // namespace ble