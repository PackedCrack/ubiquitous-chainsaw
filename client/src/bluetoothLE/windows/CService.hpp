//
// Created by qwerty on 2024-03-01.
//

#pragma once
#include "../Characteristic.hpp"


namespace ble
{
class CService
{
public:
    using awaitable_t = concurrency::task<CService>;
private:
    using GattDeviceService = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService;
public:
    [[nodiscard]] static awaitable_t make(const GattDeviceService& service);
    CService() = default;
    ~CService() = default;
    CService(const CService& other) = default;
    CService(CService&& other) noexcept = default;
    CService& operator=(const CService& other) = default;
    CService& operator=(CService&& other) = default;
private:
    explicit CService(GattDeviceService service);
public:
    [[nodiscard]] std::optional<const CCharacteristic*> characteristic(const UUID& uuid) const;
    [[nodiscard]] std::string uuid_as_str() const;
private:
    winrt::Windows::Foundation::IAsyncAction query_characteristics();
private:
    std::shared_ptr<GattDeviceService> m_pService;
    std::unordered_map<ble::UUID, CCharacteristic, ble::UUID::Hasher> m_Characteristics;
};
}   // namespace ble