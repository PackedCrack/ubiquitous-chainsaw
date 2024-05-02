#pragma once
#include "../../server_common.hpp"
#include "CGattService.hpp"
#include "profiles.hpp"
#include <memory>
// clang-format off


// clang-format on
namespace ble
{
class CWhereAmI
{
public:
    CWhereAmI();
    ~CWhereAmI() = default;
    CWhereAmI(const CWhereAmI& other);
    CWhereAmI(CWhereAmI&& other) = default;
    CWhereAmI& operator=(const CWhereAmI& other);
    CWhereAmI& operator=(CWhereAmI&& other) = default;
private:
    void copy(const CWhereAmI& other);
public:
    void register_with_nimble(const std::shared_ptr<Profile>& pProfile);
    [[nodiscard]] ble_gatt_svc_def as_nimble_service() const;
private:
    [[nodiscard]] bool valid_signature(ShaHash& hash, std::span<uint8_t> signature);
    [[nodiscard]] std::vector<CCharacteristic> make_characteristics(const std::shared_ptr<Profile>& pProfile);
    [[nodiscard]] auto make_callback_demand_rssi(const std::shared_ptr<Profile>& pProfile);
    [[nodiscard]] CCharacteristic make_characteristic_demand_rssi(const std::shared_ptr<Profile>& pProfile);
    [[nodiscard]] auto make_callback_rssi_notification(const std::shared_ptr<Profile>& pProfile);
    [[nodiscard]] CCharacteristic make_characteristic_rssi_notification(const std::shared_ptr<Profile>& pProfile);
    [[nodiscard]] std::vector<uint8_t> write_random_data_block(std::vector<uint8_t>& packet) const;
    [[nodiscard]] std::vector<uint8_t> write_rssi_value(std::vector<uint8_t>& packet) const;
    template<typename algorithm_t>
    requires security::hash_algorithm<algorithm_t>
    [[nodiscard]] std::vector<uint8_t>
        write_hash(std::vector<uint8_t>& packet, security::CHash<algorithm_t>& hash, ShaVersion version) const
    {
        static constexpr RSSINotificationHeader HEADER{};

        packet[HEADER.shaVersion] = sha_version_id(version);

        uint8_t offset = packet[HEADER.rssiOffset] + packet[HEADER.rssiSize];
        uint8_t size = hash.size();
        packet[HEADER.hashOffset] = offset;
        packet[HEADER.hashSize] = size;

        std::span<uint8_t> hashBlock{ std::begin(packet) + offset, size };
        std::size_t smallestSize = hashBlock.size() <= hash.size() ? hashBlock.size() : hash.size();
        std::memcpy(hashBlock.data(), hash.data(), smallestSize);

        return packet;
    }
    [[nodiscard]] std::vector<uint8_t> write_signature(std::vector<uint8_t>& packet, const std::vector<uint8_t>& signature) const;
private:
    std::unique_ptr<security::CEccPrivateKey> m_pPrivateKey = nullptr;
    std::unique_ptr<security::CEccPublicKey> m_pClientPublicKey = nullptr;
    CGattService m_Service;
    std::vector<CCharacteristic> m_Characteristics;
    std::vector<uint8_t> m_RandomDataBlock;
    std::optional<uint16_t> m_NotifyHandle;
    int8_t m_Rssi;
};
}    // namespace ble
