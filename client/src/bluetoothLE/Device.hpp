//
// Created by qwerty on 2024-02-14.
//
#pragma once
#ifdef WIN32
    #include "windows/CDevice.hpp"
#else
    #error Only windows is implemented atm
#endif
// clang-format off


// clang-format on
namespace ble
{
template<typename device_t, typename... make_args_t>
concept device = requires(device_t device, const device_t constDevice) {
    awaitable_make<device_t, make_args_t...>;
    typename device_t::Error;
    typename device_t::make_t;
    requires std::same_as<typename device_t::make_t, std::expected<std::shared_ptr<device_t>, typename device_t::Error>>;
    typename device_t::service_container_t;    // todo add constraints to the container

    { device_t::error_to_str(std::declval<typename device_t::Error>()) } -> std::same_as<std::string_view>;

    { constDevice.connected() } -> std::same_as<bool>;
    { constDevice.address() } -> std::same_as<uint64_t>;
    { constDevice.address_as_str() } -> std::same_as<std::string>;
    { constDevice.services() } -> std::same_as<const typename device_t::service_container_t&>;
    { constDevice.service(std::declval<const UUID>()) } -> std::same_as<std::optional<std::weak_ptr<CService>>>;
    {
        constDevice.characteristic(std::declval<const UUID>(), std::declval<const UUID>())
    } -> std::same_as<std::optional<std::weak_ptr<CCharacteristic>>>;
};
template<typename device_t, typename... ctor_args_t>
requires device<device_t, ctor_args_t...>
[[nodiscard]] typename device_t::awaitable_make_t make_device(ctor_args_t&&... args)
{
    return device_t::make(std::forward<ctor_args_t>(args)...);
}
}    // namespace ble
