//
// Created by qwerty on 2024-03-01.
//

#pragma once
#include "CCharacteristic.hpp"



namespace ble::win
{
class CService
{
public:
    [[nodiscard]] static CService make_service(
            const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService& service);
    ~CService() = default;
    CService(const CService& other) = default;
    CService(CService&& other) = default;
    CService& operator=(const CService& other) = default;
    CService& operator=(CService&& other) = default;
private:
    explicit CService(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService service);
    winrt::Windows::Foundation::IAsyncAction init();
public:
    winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService m_Service;
    std::unordered_map<ble::UUID, CCharacteristic, ble::UUID::Hasher> m_Characteristics;
};
}   // namespace ble::win