#pragma once
// std
#include <cstdint>
#include <concepts>


namespace ble
{
struct UUID
{
    std::array<uint8_t, 16u> data{};
    
    static constexpr void apply_custom_id(UUID& uuid, uint16_t id)
    {
        uuid.data[2] = (id & 0xFF00) >> 8;
        uuid.data[3] = id & 0x00FF;
    }
    [[nodiscard]] friend bool operator==(const UUID& lhs, const UUID& rhs)
    {
        return lhs.data == rhs.data;
    }
    [[nodiscard]] friend bool operator!=(const UUID& lhs, const UUID& rhs)
    {
        return lhs.data != rhs.data;
    }
    struct Hasher
    {
        [[nodiscard]] std::size_t operator()(const UUID& uuid) const
        {
            static constexpr std::size_t prime = 31u;
            std::hash<uint8_t> hasher{};
            
            size_t hash{};
            for(auto&& byte : uuid.data)
                hash = (hash + hasher(byte)) * prime;
            
            return hash;
        }
    };
};
static constexpr UUID BaseUUID
{
        .data = { 0x00, 0x00,
                  0x00, 0x00,
                  0x00, 0x00,
                  0x10, 0x00,
                  0x80, 0x00,
                  0x00, 0x80,
                  0x5F, 0x9b,
                  0x34, 0xFB }
};
[[nodiscard]] consteval UUID client_characteristic_configuration_descriptor()
{
    UUID uuid = BaseUUID;
    UUID::apply_custom_id(uuid, 0x2902);
    
    return uuid;
}
static constexpr uint16_t ID_SERVICE_WHOAMI = 0xBABE;
static constexpr uint16_t ID_CHARACTERISTIC_WHOAMI_AUTHENTICATE = 0xB00B;
enum class HashType : uint8_t
{
    Sha2_224 = 0u,
    Sha2_256 = 1u,
    Sha3_224 = 2u,
    Sha3_256 = 3u,
    Sha3_384 = 4u,
    Sha3_512 = 5u,
    count
};
/**
 * @brief Represents the header for server authentication.
 *
 * This structure is designed to hold information about the authentication data
 * within a server packet or similar context. It includes details about both
 * hash and signature blocks contained within the packet.
 */
struct ServerAuthHeader
{
    uint8_t hashType = 0;         ///< Enum value for the type of hash used.
    uint8_t hashOffset = 1u;      ///< Offset where the hash data begins.
    uint8_t hashSize = 2u;        ///< Size of the hash data in bytes.
    uint8_t signatureOffset = 3u; ///< Offset where the signature data begins.
    uint8_t signatureSize = 4u;   ///< Size of the signature data in bytes.
};
[[nodiscard]] consteval ServerAuthHeader whoami_server_auth_header()
{
    return ServerAuthHeader{};
}
[[nodiscard]] consteval UUID uuid_service_whoami()
{
    UUID uuid = BaseUUID;
    UUID::apply_custom_id(uuid, ID_SERVICE_WHOAMI);
    return uuid;
}
[[nodiscard]] consteval UUID uuid_characteristic_whoami_authenticate()
{
    UUID uuid = BaseUUID;
    UUID::apply_custom_id(uuid, ID_CHARACTERISTIC_WHOAMI_AUTHENTICATE);
    return uuid;
}
static constexpr uint16_t ID_SERVICE_WHEREAMI = 0xFEED;
static constexpr uint16_t ID_CHARACTERISTIC_WHEREAMI_DEMAND_RSSI = 0xBEEF;
static constexpr uint16_t ID_CHARACTERISTIC_WHEREAMI_SEND_RSSI = 0xCAFE;
[[nodiscard]] consteval UUID uuid_service_whereami()
{
    UUID uuid = BaseUUID;
    UUID::apply_custom_id(uuid, ID_SERVICE_WHEREAMI);
    return uuid;
}
[[nodiscard]] consteval UUID uuid_characteristic_whereami_demand_rssi()
{
    UUID uuid = BaseUUID;
    UUID::apply_custom_id(uuid, ID_CHARACTERISTIC_WHEREAMI_DEMAND_RSSI);
    return uuid;
}
[[nodiscard]] consteval UUID uuid_characteristic_whereami_send_rssi()
{
    UUID uuid = BaseUUID;
    UUID::apply_custom_id(uuid, ID_CHARACTERISTIC_WHEREAMI_SEND_RSSI);
    return uuid;
}
}   // namespace ble