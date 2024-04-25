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
concept device = requires(device_t device, const device_t constDevice, const UUID uuid) {
    awaitable_make<device_t, make_args_t...>;
    typename device_t::Error;
    typename device_t::make_t;
    requires std::same_as<typename device_t::make_t, std::expected<device_t, typename device_t::Error>>;
    typename device_t::service_container_t;    // todo add constraints to the container

    {
        device.set_connection_changed_cb([](ConnectionStatus) {})
    };
    { device.set_connection_changed_cb(std::function<void(ConnectionStatus)>{}) };

    { constDevice.connected() } -> std::same_as<bool>;
    { constDevice.address() } -> std::same_as<uint64_t>;
    { constDevice.address_as_str() } -> std::convertible_to<std::string>;
    { constDevice.services() } -> std::convertible_to<const typename device_t::service_container_t&>;
    { constDevice.service(uuid) } -> std::convertible_to<std::optional<const CService*>>;
};
template<typename device_t, typename... ctor_args_t>
requires device<device_t, ctor_args_t...>
[[nodiscard]] typename device_t::awaitable_t make_device(ctor_args_t&&... args)
{
    return device_t::make(std::forward<ctor_args_t>(args)...);
}
}    // namespace ble
