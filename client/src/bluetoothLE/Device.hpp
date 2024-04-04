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
concept Device = requires(const device_t constDevice, const UUID uuid)
{
    awaitable_make<device_t, make_args_t...>;
    typename device_t::service_container_t; // todo add constraints to the container
    
    { constDevice.address() } -> std::convertible_to<uint64_t>;
    { constDevice.address_as_str() } -> std::convertible_to<std::string>;
    { constDevice.services() } -> std::convertible_to<const typename device_t::service_container_t&>;
    { constDevice.service(uuid) } -> std::convertible_to<std::optional<const CService*>>;
};

template<typename device_t, typename... ctor_args_t>
requires Device<device_t, ctor_args_t...>
[[nodiscard]] typename device_t::awaitable_t make_device(ctor_args_t&&... args)
{
    return device_t::make(std::forward<ctor_args_t>(args)...);
}
}   // namespace ble