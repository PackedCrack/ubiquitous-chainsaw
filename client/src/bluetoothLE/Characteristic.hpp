//
// Created by qwerty on 2024-04-02.
//

#pragma once
#ifdef WIN32
    #include "windows/CCharacteristic.hpp"
#else
    #error Only windows is implemented atm
#endif
#include "common/common.hpp"
#include "ble_common.hpp"
namespace ble
{
template<typename expected_t>
concept expected_like = requires(expected_t expected) {
    typename expected_t::value_type;
    typename expected_t::error_type;
    { expected.value() } -> std::convertible_to<typename expected_t::value_type&>;
    { expected.error() } -> std::convertible_to<typename expected_t::error_type&>;
    { static_cast<bool>(expected) } -> std::convertible_to<bool>;
    requires std::is_pointer_v<decltype(expected.operator->())>;
    requires std::is_lvalue_reference_v<decltype(expected.operator*())>;
    //{ expected.operator->() } -> std::convertible_to<typename expected_t::value_type*>;
    { expected.operator*() } -> std::convertible_to<typename expected_t::value_type&>;
};
template<typename characteristic_t, typename... make_args_t>
concept characteristic =
    requires(characteristic_t characteristic, const characteristic_t constCharacteristic, const std::vector<uint8_t>& data) {
        awaitable_make<characteristic_t, make_args_t...>;
        string_uuid<characteristic_t>;
        // Required type alias
        typename characteristic_t::read_t;
        requires expected_like<typename characteristic_t::read_t>;
        requires common::buffer<typename characteristic_t::read_t::value_type>;
        typename characteristic_t::awaitable_read_t;
        typename characteristic_t::awaitable_communication_status_t;
        typename characteristic_t::awaitable_subscription_state_t;

        // Required public functions
        {
            characteristic.subscribe_to_notify([](std::span<const uint8_t>) {})
        } -> std::same_as<typename characteristic_t::awaitable_subscription_state_t>;
        {
            characteristic.subscribe_to_notify(std::function<void(std::span<const uint8_t>)>{})
        } -> std::same_as<typename characteristic_t::awaitable_subscription_state_t>;
        { characteristic.unsubscribe() } -> std::same_as<typename characteristic_t::awaitable_subscription_state_t>;

        // Required public const functions
        { constCharacteristic.uuid_as_str() } -> std::same_as<std::string>;
        { constCharacteristic.read_value() } -> std::same_as<typename characteristic_t::awaitable_read_t>;
        { constCharacteristic.write_data(data) } -> std::same_as<typename characteristic_t::awaitable_communication_status_t>;
        { constCharacteristic.write_data_with_response(data) } -> std::same_as<typename characteristic_t::awaitable_communication_status_t>;
        { constCharacteristic.properties() } -> std::same_as<CharacteristicProperties>;
        { constCharacteristic.properties_as_str() } -> std::same_as<std::vector<std::string>>;
        { constCharacteristic.protection_level() } -> std::same_as<ProtectionLevel>;
        { constCharacteristic.has_subscribed() } -> std::same_as<typename characteristic_t::awaitable_subscription_state_t>;
    };
template<typename characteristic_t, typename... ctor_args_t>
requires characteristic<characteristic_t, ctor_args_t...>
[[nodiscard]] typename characteristic_t::awaitable_make_t make_characteristic(ctor_args_t&&... args)
{
    return characteristic_t::make(std::forward<ctor_args_t>(args)...);
}
}    // namespace ble
