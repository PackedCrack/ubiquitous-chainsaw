//
// Created by qwerty on 2024-01-26.
//
#pragma once
#include "../client_defines.hpp"
#include "common/ble_services.hpp"
#include "common/common.hpp"
// clang-format off


// clang-format on
namespace ble
{
template<typename int_t>
concept unsigned_integral = std::integral<int_t> && std::is_unsigned_v<int_t>;
template<typename async_t, typename... make_args_t>
concept awaitable_make = requires {
    typename async_t::awaitable_make_t;
    requires std::is_invocable_r_v<typename async_t::awaitable_make_t, decltype(&async_t::make), make_args_t...>;
};
template<typename T>
concept string_uuid = requires(const T type) {
    { type.uuid_as_str() } -> std::same_as<std::string>;
};
template<typename int_t>
requires unsigned_integral<int_t>
[[nodiscard]] uint8_t lsb_of_uint(int_t value)
{
    return static_cast<uint8_t>(value);
}
template<typename uuid_t>
[[nodiscard]] ble::UUID make_uuid(uuid_t&& guid)
{
    ble::UUID uuid{};
    uuid.data[0] = (guid.Data1 & 0xFF'00'00'00) >> 24;
    uuid.data[1] = common::assert_down_cast<uint8_t>((guid.Data1 & 0x00'FF'00'00) >> 16);
    uuid.data[2] = (guid.Data1 & 0x00'00'FF'00) >> 8;
    uuid.data[3] = (guid.Data1 & 0x00'00'00'FF);

    uuid.data[4] = (guid.Data2 & 0x00'00'FF'00) >> 8;
    uuid.data[5] = (guid.Data2 & 0x00'00'00'FF);

    uuid.data[6] = (guid.Data3 & 0x00'00'FF'00) >> 8;
    uuid.data[7] = (guid.Data3 & 0x00'00'00'FF);

    uuid.data[8] = guid.Data4[0];
    uuid.data[9] = guid.Data4[1];
    uuid.data[10] = guid.Data4[2];
    uuid.data[11] = guid.Data4[3];
    uuid.data[12] = guid.Data4[4];
    uuid.data[13] = guid.Data4[5];
    uuid.data[14] = guid.Data4[6];
    uuid.data[15] = guid.Data4[7];

#ifndef NDEBUG
    ASSERT((uuid == uuid_service_whoami()) || (uuid == uuid_characteristic_whoami_authenticate()) || (uuid == uuid_service_whereami()) ||
               (uuid == uuid_characteristic_whereami_rssi_notification()) || (uuid == uuid_characteristic_whereami_demand_rssi()) ||
               (uuid == uuid_descriptor_client_characteristic_configuration_descriptor()),
           "Unknown uuid!");
#endif

    return uuid;
}
[[nodiscard]] inline char byte_to_hex_char(size_t index)
{
    static constexpr std::array<char, 16u> hexValues{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    return hexValues[index];
}
[[nodiscard]] inline char low_nibble_as_char(uint8_t byte)
{
    return byte_to_hex_char(byte & 0x0F);
}
[[nodiscard]] inline char high_nibble_as_char(uint8_t byte)
{
    return byte_to_hex_char(byte >> 4u);
}
[[nodiscard]] inline std::string hex_addr_to_str(uint64_t addr)
{
    std::string address(17, ':');

    for (auto i = static_cast<int64_t>(address.size() - 1u); i >= 0u;)
    {
        uint8_t byte = lsb_of_uint(addr);
        address[i] = low_nibble_as_char(byte);
        address[i - 1u] = high_nibble_as_char(byte);
        addr = addr >> 8u;

        i = i - 3u;
    }

    return address;
}
enum class AddressType
{
    real,
    random,
    none
};
[[nodiscard]] constexpr std::string_view address_type_to_str(AddressType type)
{
    // clang-format off
    UNHANDLED_CASE_PROTECTION_ON
    switch (type)
    {
    case AddressType::real: { return "Real"; }
    case AddressType::random: { return "Random"; }
    case AddressType::none: { return "None"; }
    }
    UNHANDLED_CASE_PROTECTION_OFF
    // clang-format on

    std::unreachable();
}
struct DeviceInfo
{
    std::optional<uint64_t> address;
    AddressType addressType = AddressType::none;
    [[nodiscard]] static std::string address_as_str(uint64_t address) { return hex_addr_to_str(address); }
};
enum class ConnectionStatus
{
    connected = 0,
    disconnected = 1
};
[[nodiscard]] constexpr std::string_view connnection_status_to_str(ConnectionStatus status)
{
    // clang-format off
    UNHANDLED_CASE_PROTECTION_ON
    switch (status)
    // cppcheck-suppress missingReturn
    {
    case ConnectionStatus::connected: return "Connected";
    case ConnectionStatus::disconnected: return "Disconnected";
    }
    UNHANDLED_CASE_PROTECTION_OFF
    // clang-format on

    std::unreachable();
}
enum class CommunicationStatus
{
    success = 0,
    unreachable = 1,
    protocolError = 2,
    accessDenied = 3
};
[[nodiscard]] constexpr std::string_view communication_status_to_str(CommunicationStatus status)
{
    // clang-format off
    UNHANDLED_CASE_PROTECTION_ON
    switch (status)
    // cppcheck-suppress missingReturn
    {
    case CommunicationStatus::unreachable: return "Unreachable";
    case CommunicationStatus::protocolError: return "Protocol Error";
    case CommunicationStatus::accessDenied: return "Access Denied";
    case CommunicationStatus::success: return "Success";
    }
    UNHANDLED_CASE_PROTECTION_OFF
    // clang-format on

    std::unreachable();
}
enum class CharacteristicProperties : uint32_t
{
    none = 0,
    broadcast = 0x1,
    read = 0x2,
    writeWithoutResponse = 0x4,
    write = 0x8,
    notify = 0x10,
    indicate = 0x20,
    authenticatedSignedWrites = 0x40,
    extendedProperties = 0x80,
    reliableWrites = 0x1'00,
    writableAuxiliaries = 0x2'00,
};
[[nodiscard]] constexpr CharacteristicProperties operator&(CharacteristicProperties lhs, CharacteristicProperties rhs)
{
    return CharacteristicProperties{ std::to_underlying(lhs) & std::to_underlying(rhs) };
}
[[nodiscard]] constexpr CharacteristicProperties operator|(CharacteristicProperties lhs, CharacteristicProperties rhs)
{
    return CharacteristicProperties{ std::to_underlying(lhs) | std::to_underlying(rhs) };
}
[[nodiscard]] constexpr CharacteristicProperties operator^(CharacteristicProperties lhs, CharacteristicProperties rhs)
{
    return CharacteristicProperties{ std::to_underlying(lhs) ^ std::to_underlying(rhs) };
}
[[nodiscard]] constexpr CharacteristicProperties operator~(CharacteristicProperties prop)
{
    return CharacteristicProperties{ std::to_underlying(prop) };
}
[[nodiscard]] constexpr std::string_view characteristic_properties_to_str(CharacteristicProperties properties)
{
    // clang-format off
    UNHANDLED_CASE_PROTECTION_ON
    switch (properties)
    // cppcheck-suppress missingReturn
    {
    case CharacteristicProperties::none: return "None";
    case CharacteristicProperties::broadcast: return "Broadcast";
    case CharacteristicProperties::read: return "Read";
    case CharacteristicProperties::writeWithoutResponse: return "Write Without Response";
    case CharacteristicProperties::write: return "Write";
    case CharacteristicProperties::notify: return "Notify";
    case CharacteristicProperties::indicate: return "Indicate";
    case CharacteristicProperties::authenticatedSignedWrites: return "Authenticated Signed Writes";
    case CharacteristicProperties::extendedProperties: return "Extended Properties";
    case CharacteristicProperties::reliableWrites: return "Reliable Writes";
    case CharacteristicProperties::writableAuxiliaries: return "Writable Auxiliaries";
    }
    UNHANDLED_CASE_PROTECTION_OFF
    // clang-format on

    std::unreachable();
}
enum class ProtectionLevel : int32_t
{
    plain = 0,
    authenticationRequired = 1,
    encryptionRequired = 2,
    encryptionAndAuthenticationRequired = 3,
};
[[nodiscard]] constexpr std::string_view prot_level_to_str(ProtectionLevel level)
{
    // clang-format off
    UNHANDLED_CASE_PROTECTION_ON
    switch (level)
    // cppcheck-suppress missingReturn
    {
    case ProtectionLevel::authenticationRequired: return "Authentication Required";
    case ProtectionLevel::encryptionAndAuthenticationRequired: return "Encryption and Authentication Required";
    case ProtectionLevel::encryptionRequired: return "Encryption Required";
    case ProtectionLevel::plain: return "Plain";
    }
    UNHANDLED_CASE_PROTECTION_OFF
    // clang-format off
    
    std::unreachable();
}
}    // namespace ble