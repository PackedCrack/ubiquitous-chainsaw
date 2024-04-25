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
    [[nodiscard]] auto make_callback_send_rssi(const std::shared_ptr<Profile>& pProfile);
    [[nodiscard]] CCharacteristic make_characteristic_send_rssi(const std::shared_ptr<Profile>& pProfile);
private:
    int8_t m_Rssi;
    std::optional<uint16_t> m_NotifyHandle;
    std::unique_ptr<security::CEccPrivateKey> m_pPrivateKey = nullptr;
    std::unique_ptr<security::CEccPublicKey> m_pClientPublicKey = nullptr;
    std::vector<CCharacteristic> m_Characteristics;
    CGattService m_Service;
};
}    // namespace ble
