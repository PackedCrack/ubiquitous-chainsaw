#pragma once
#include "../../server_common.hpp"
#include "CGattService.hpp"
#include "profiles.hpp"
#include <memory>

namespace ble
{
class CWhoAmI
{
public:
    CWhoAmI();
    ~CWhoAmI() = default;
    CWhoAmI(const CWhoAmI& other);
    CWhoAmI(CWhoAmI&& other) = default;
    CWhoAmI& operator=(const CWhoAmI& other);
    CWhoAmI& operator=(CWhoAmI&& other) = default;
private:
    void copy(const CWhoAmI& other);
public:
    void register_with_nimble(const std::shared_ptr<Profile>& pProfile);
    [[nodiscard]] ble_gatt_svc_def as_nimble_service() const;
private:
    void sign_server_mac_address();
    [[nodiscard]] std::vector<CCharacteristic> make_characteristics(const std::shared_ptr<Profile>& pProfile);
    [[nodiscard]] auto make_callback_authenticate(const std::shared_ptr<Profile>& pProfile);
    [[nodiscard]] CCharacteristic make_characteristic_authenticate(const std::shared_ptr<Profile>& pProfile);
private:
    std::unique_ptr<security::CEccPrivateKey> m_pPrivateKey = nullptr;
    std::vector<security::byte> m_SignedMacData;
    std::vector<CCharacteristic> m_Characteristics;
    CGattService m_Service;
};
}   // namespace ble