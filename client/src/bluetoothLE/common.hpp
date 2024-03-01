//
// Created by qwerty on 2024-01-26.
//

#pragma once
#include "defines.hpp"
#include "../common/ble_services.hpp"


namespace ble
{
template<typename int_t>
concept unsigned_integral = std::integral<int_t> && std::is_unsigned_v<int_t>;

template<typename int_t> requires unsigned_integral<int_t>
[[nodiscard]] uint8_t lsb_of_uint(int_t value)
{
    return static_cast<uint8_t>(value);
}

template<typename uuid_t>
[[nodiscard]] ble::UUID make_uuid(uuid_t&& guid)
{
    ble::UUID uuid{};
    static_assert(std::is_trivially_copyable_v<std::remove_reference_t<decltype(guid)>>);
    static_assert(std::is_trivially_copy_constructible_v<ble::UUID>);
    static_assert(sizeof(ble::UUID) == sizeof(std::remove_reference_t<decltype(guid)>));
    std::memcpy(&uuid, &guid, sizeof(uuid));
    
    return uuid;
}

[[nodiscard]] inline char byte_to_hex_char(size_t index)
{
    static constexpr std::array<char, 16u> hexValues{
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    return hexValues[ index ];
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
    
    for(auto i = static_cast<int64_t>(address.size() - 1u); i >= 0u;)
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

[[nodiscard]] inline std::string_view address_type_to_str(AddressType type)
{
    UNHANDLED_CASE_PROTECTION_ON
    switch(type)
    {
        case AddressType::real:
        {
            static constexpr std::string_view asStr = "Real";
            return asStr;
        }
        case AddressType::random:
        {
            static constexpr std::string_view asStr = "Random";
            return asStr;
        }
        case AddressType::none:
        {
            static constexpr std::string_view asStr = "None";
            return asStr;
        }
    }
    UNHANDLED_CASE_PROTECTION_OFF
    
    std::unreachable();
}

struct DeviceInfo
{
    std::optional<uint64_t> address;
    AddressType addressType = AddressType::none;
    
    [[nodiscard]] static std::string address_as_str(uint64_t address)
    {
        return hex_addr_to_str(address);
    }
};
}   // namespace ble