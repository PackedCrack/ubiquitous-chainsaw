#pragma once
#include "../security/sha.hpp"
// std
#include <cstdint>
#include <concepts>
// clang-format off


// clang-format on
namespace ble
{
struct UUID
{
    std::array<uint8_t, 16u> data{};
    static constexpr void apply_custom_id(UUID& uuid, uint16_t id)
    {
        uuid.data[2] = static_cast<uint8_t>((id & 0xFF'00) >> 8);
        uuid.data[3] = static_cast<uint8_t>(id & 0x00'FF);
    }
    [[nodiscard]] friend bool operator==(const UUID& lhs, const UUID& rhs) { return lhs.data == rhs.data; }
    [[nodiscard]] friend bool operator!=(const UUID& lhs, const UUID& rhs) { return lhs.data != rhs.data; }
    struct Hasher
    {
        [[nodiscard]] std::size_t operator()(const UUID& uuid) const
        {
            static constexpr std::size_t prime = 31u;
            std::hash<uint8_t> hasher{};

            size_t hash{};
            for (auto&& byte : uuid.data)
            {
                hash = (hash + hasher(byte)) * prime;
            }

            return hash;
        }
    };
};
static constexpr UUID BaseUUID{
    // clang-format off
        .data = { 0x00, 0x00,
                  0x00, 0x00,
                  0x00, 0x00,
                  0x10, 0x00,
                  0x80, 0x00,
                  0x00, 0x80,
                  0x5F, 0x9b,
                  0x34, 0xFB }
    // clang-format on
};
[[nodiscard]] consteval UUID client_characteristic_configuration_descriptor()
{
    UUID uuid = BaseUUID;
    UUID::apply_custom_id(uuid, 0x29'02);

    return uuid;
}
static constexpr uint16_t ID_SERVICE_WHOAMI = 0xBA'BE;
static constexpr uint16_t ID_CHARACTERISTIC_WHOAMI_AUTHENTICATE = 0xB0'0B;
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
template<typename sha_t>
[[nodiscard]] constexpr HashType sha_to_enum()
{
    // clang-format off
    if constexpr(std::same_as<sha_t, security::Sha2_224>) { return HashType::Sha2_224; }
    else if constexpr(std::same_as<sha_t, security::Sha2_256>) { return HashType::Sha2_256; }
    else if constexpr(std::same_as<sha_t, security::Sha3_224>) { return HashType::Sha3_224; }
    else if constexpr(std::same_as<sha_t, security::Sha3_256>) { return HashType::Sha3_256; }
    else if constexpr(std::same_as<sha_t, security::Sha3_384>) { return HashType::Sha3_384; }
    else
    { static_assert(std::same_as<sha_t, security::Sha3_512>, "Type unsupported"); return HashType::Sha3_512; }
    // clang-format on
}
[[nodiscard]] constexpr uint8_t hash_type_id(HashType type)
{
    UNHANDLED_CASE_PROTECTION_ON
    switch (type)
    {
        // clang-format off
        case ble::HashType::Sha2_224: return std::to_underlying(ble::HashType::Sha2_224);
        case ble::HashType::Sha2_256: return std::to_underlying(ble::HashType::Sha2_256);
        case ble::HashType::Sha3_224: return std::to_underlying(ble::HashType::Sha3_224);
        case ble::HashType::Sha3_256: return std::to_underlying(ble::HashType::Sha3_256);
        case ble::HashType::Sha3_384: return std::to_underlying(ble::HashType::Sha3_384);
        case ble::HashType::Sha3_512: return std::to_underlying(ble::HashType::Sha3_512);
        case ble::HashType::count:
            // cppcheck-suppress constStatement
            ASSERT(false, "Value of \"count\" passed unexpectedly from ble::HashType"); // this should never happen so we want to break here..
            return 0;
        // clang-format on
    }
    UNHANDLED_CASE_PROTECTION_OFF

    std::unreachable();
}
/**
 * @brief Represents the header for server authentication.
 *
 * This structure holds information about the packet header for whoami's
 * authentication characteristic. Each member hold the byte offset into
 * the packet where the information should be stored. The type of the member
 * indicate how many bytes should be reserved for that information.
 */
struct AuthenticateHeader
{
    uint8_t hashType = 0;            ///< Enum value for the type of hash used.
    uint8_t hashOffset = 1u;         ///< Offset where the hash data begins.
    uint8_t hashSize = 2u;           ///< Size of the hash data in bytes.
    uint8_t signatureOffset = 3u;    ///< Offset where the signature data begins.
    uint8_t signatureSize = 4u;      ///< Size of the signature data in bytes.
};
[[nodiscard]] consteval AuthenticateHeader header_whoami_authenticate()
{
    return AuthenticateHeader{};
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
static constexpr uint16_t ID_SERVICE_WHEREAMI = 0xFE'ED;
static constexpr uint16_t ID_CHARACTERISTIC_WHEREAMI_DEMAND_RSSI = 0xBE'EF;
static constexpr uint16_t ID_CHARACTERISTIC_WHEREAMI_SEND_RSSI = 0xCA'FE;
struct DemandRSSIHeader
{
    uint8_t randomDataOffset = 0;
    uint8_t randomDataSize = 1;
    uint8_t hashType = 2;
    uint8_t hashOffset = 3u;
    uint8_t hashSize = 4u;
    uint8_t signatureOffset = 5u;
    uint8_t signatureSize = 6u;
};
[[nodiscard]] consteval DemandRSSIHeader header_whereami_demand_rssi()
{
    return DemandRSSIHeader{};
}
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
}    // namespace ble
