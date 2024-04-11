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
template<typename async_t, typename... make_args_t>
concept awaitable_make = requires
{
    typename async_t::awaitable_t;
    requires std::is_invocable_r_v<typename async_t::awaitable_t, decltype(&async_t::make), make_args_t...>;
};
template<typename T>
concept string_uuid = requires(const T type)
{
    { type.uuid_as_str() } -> std::convertible_to<std::string>;
};

template<typename int_t> requires unsigned_integral<int_t>
[[nodiscard]] uint8_t lsb_of_uint(int_t value)
{
    return static_cast<uint8_t>(value);
}
template<typename uuid_t>
[[nodiscard]] ble::UUID make_uuid(uuid_t&& guid)
{
    // https://stackoverflow.com/questions/9195551/why-does-guid-tobytearray-order-the-bytes-the-way-it-does
    ble::UUID uuid{};

    uuid.data7 = static_cast<uint16_t>(guid.Data1 >> 16);
    uuid.data6 = static_cast<uint16_t>(guid.Data1 & 0x0000'FFFF);
    
    uuid.data5 = guid.Data2;
    uuid.data4 = guid.Data3;
    
    uuid.data3 = static_cast<uint16_t>(guid.Data4[0] << 8);
    uuid.data3 = uuid.data3 | static_cast<uint16_t>(guid.Data4[1]);
    
    uuid.custom = static_cast<uint16_t>(guid.Data4[5] << 8);
    uuid.custom = uuid.custom | static_cast<uint16_t>(guid.Data4[4]);
    
    uuid.data1 = static_cast<uint32_t>(guid.Data4[7] << 24);
    uuid.data1 = uuid.data1 | static_cast<uint32_t>(guid.Data4[6] << 16);
    uuid.data1 = uuid.data1 | static_cast<uint32_t>(guid.Data4[3] << 8);
    uuid.data1 = uuid.data1 | static_cast<uint32_t>(guid.Data4[2]);

#ifndef NDEBUG
    ble::UUID whoami{ ble::BaseUUID };
    whoami.custom = ble::ID_SERVICE_WHOAMI;
    ble::UUID serverAuth{ ble::BaseUUID };
    serverAuth.custom = ble::ID_CHARACTERISTIC_SERVER_AUTH;
    ble::UUID clientAuth{ ble::BaseUUID };
    clientAuth.custom = ble::ID_CHARACTERISTIC_CLIENT_AUTH;
    
    ASSERT((uuid == whoami) || (uuid == serverAuth) || (uuid == clientAuth), "Unknown uuid!");
#endif
    
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