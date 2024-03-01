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
    enum class State : uint32_t
    {
        uninitialized,
        queryingCharacteristics,
        ready
    };
public:
    [[nodiscard]] static CService make_service(
            const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService& service);
    ~CService() = default;
    CService(const CService& other) = default;
    CService(CService&& other) = default;
    CService& operator=(const CService& other) = default;
    CService& operator=(CService&& other) = default;
public:
    [[nodiscard]] std::string uuid_as_str() const;
    [[nodiscard]] bool ready() const;
    [[nodiscard]] State state() const;
    winrt::Windows::Foundation::IAsyncAction query_characteristics();
private:
    explicit CService(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService service);
    winrt::Windows::Foundation::IAsyncAction init();
public:
    winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService m_Service;
    std::unordered_map<ble::UUID, CCharacteristic, ble::UUID::Hasher> m_Characteristics;
    State m_State;
};
}   // namespace ble::win