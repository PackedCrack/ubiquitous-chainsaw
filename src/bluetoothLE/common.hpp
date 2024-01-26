//
// Created by qwerty on 2024-01-26.
//

#pragma once


namespace ble
{
template<typename int_t>
concept unsigned_integral = std::integral<int_t> && std::is_unsigned_v<int_t>;

template<typename int_t> requires unsigned_integral<int_t>
[[nodiscard]] uint8_t lsb_of_uint(int_t value)
{
    return static_cast<uint8_t>(value);
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

struct DeviceInfo
{
    std::optional<std::string> address;
    AddressType addressType = AddressType::none;
};
}   // namespace ble