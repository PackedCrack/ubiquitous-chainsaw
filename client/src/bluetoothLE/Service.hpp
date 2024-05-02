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
// clang-format off


// clang-format on
namespace ble
{
template<typename service_t, typename... make_args_t>
concept service = requires(service_t service, const service_t constService, const UUID uuid) {
    awaitable_make<service_t, make_args_t...>;
    string_uuid<service_t>;
    //typename service_t::awaitable_subscribe_t;

    //{
    //    service.subscribe_to_characteristic(UUID{}, [](std::span<const uint8_t>) {})
    //} -> std::same_as<typename service_t::awaitable_subscribe_t>;
    //{
    //    service.subscribe_to_characteristic(UUID{}, std::function<void(std::span<const uint8_t>)>{})
    //} -> std::same_as<typename service_t::awaitable_subscribe_t>;
    //{ service.unsubscribe_from_characteristic(UUID{}) };

    { constService.characteristic(uuid) } -> std::same_as<std::optional<std::weak_ptr<CCharacteristic>>>;
};
template<typename service_t, typename... ctor_args_t>
requires service<service_t, ctor_args_t...>
[[nodiscard]] typename service_t::awaitable_make_t make_service(ctor_args_t&&... args)
{
    return service_t::make(std::forward<ctor_args_t>(args)...);
}
}    // namespace ble
