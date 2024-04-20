#pragma once
// std
#include <cstdint>
#include <concepts>


namespace ble
{
struct UUID
{
    uint32_t data1 = 0u;
    uint16_t custom = 0u;
    uint16_t data3 = 0u;
    uint16_t data4 = 0u;
    uint16_t data5 = 0u;
    uint16_t data6 = 0u;
    uint16_t data7 = 0u;
    
    
    [[nodiscard]] friend bool operator==(const UUID& lhs, const UUID& rhs)
    {
        if(lhs.data1 != rhs.data1)
            return false;
        if(lhs.custom != rhs.custom)
            return false;
        if(lhs.data3 != rhs.data3)
            return false;
        if(lhs.data4 != rhs.data4)
            return false;
        if(lhs.data5 != rhs.data5)
            return false;
        if(lhs.data6 != rhs.data6)
            return false;
        if(lhs.data7 != rhs.data7)
            return false;
        
        return true;
    }
    [[nodiscard]] friend bool operator!=(const UUID& lhs, const UUID& rhs)
    {
        if(lhs.data1 == rhs.data1 &&
        lhs.custom == rhs.custom &&
        lhs.data3 == rhs.data3 &&
        lhs.data4 == rhs.data4 &&
        lhs.data5 == rhs.data5 &&
        lhs.data6 == rhs.data6 &&
        lhs.data7 == rhs.data7)
        {
            return true;
        }
        
            return false;
    }
    struct Hasher
    {
        [[nodiscard]] std::size_t operator()(const UUID& uuid) const
        {
            static constexpr std::size_t prime = 31u;
            
            std::size_t hash = 1u;
            hash = (hash * prime) + (uuid.data1 ^ lower_end_bits(uuid.data1));
            hash = (hash * prime) + (uuid.custom ^ lower_end_bits(uuid.custom));
            hash = (hash * prime) + (uuid.data3 ^ lower_end_bits(uuid.data3));
            hash = (hash * prime) + (uuid.data4 ^ lower_end_bits(uuid.data4));
            hash = (hash * prime) + (uuid.data5 ^ lower_end_bits(uuid.data5));
            hash = (hash * prime) + (uuid.data6 ^ lower_end_bits(uuid.data6));
            hash = (hash * prime) + (uuid.data7 ^ lower_end_bits(uuid.data7));
            
            return hash;
        }
    private:
        template<typename integer_t> requires std::integral<integer_t>
        [[nodiscard]] std::size_t lower_end_bits(integer_t val) const
        {
            return val >> (sizeof(decltype(data1)) / 2);
        }
    };
};

static constexpr UUID BaseUUID
{
        .data1 = 0u,
        .custom = 0u,
        .data3 = 0x1000,
        .data4 = 0x8000,
        .data5 = 0x0080,
        .data6 = 0x5F9B,
        .data7 = 0x34FB
};

static constexpr uint16_t ID_SERVICE_WHOAMI = 0xBEBA;
static constexpr uint16_t ID_CHARACTERISTIC_SERVER_AUTH = 0x0BB0;

static constexpr uint16_t ID_SERVICE_WHEREAMI = 0xEDFE;
static constexpr uint16_t ID_CHARACTERISTIC_CLIENT_QUERY = 0xEFBE;
static constexpr uint16_t ID_CHARACTERISTIC_CLIENT_NOTIFY = 0xFECA;

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
    uuid.custom = ID_SERVICE_WHOAMI;
    return uuid;
}
[[nodiscard]] consteval UUID uuid_characteristic_server_auth()
{
    UUID uuid = BaseUUID;
    uuid.custom = ID_CHARACTERISTIC_SERVER_AUTH;
    return uuid;
}
[[nodiscard]] consteval UUID uuid_characteristic_client_auth()
{
    UUID uuid = BaseUUID;
    uuid.custom = ID_CHARACTERISTIC_CLIENT_AUTH;
    return uuid;
}

}   // namespace ble