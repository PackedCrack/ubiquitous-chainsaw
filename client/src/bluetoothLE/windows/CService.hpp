//
// Created by qwerty on 2024-03-01.
//

#pragma once
#include "CCharacteristic.hpp"
#include <pplawait.h>


namespace ble
{
class CService
{
public:
    using awaitable_t = concurrency::task<CService>;
    enum class State : uint32_t
    {
        uninitialized,
        queryingCharacteristics,
        ready
    };
    enum class Error
    {
        characteristicNotFound
    };
private:
    using GattDeviceService = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService;
public:
    CService() = default;
    [[nodiscard]] static awaitable_t make(
            const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService& service);
    ~CService() = default;
    CService(const CService& other) = default;
    CService(CService&& other) = default;
    CService& operator=(const CService& other) = default;
    CService& operator=(CService&& other) = default;
public:
    [[nodiscard]] std::expected<const CCharacteristic*, Error> characteristic(const UUID& uuid) const;
    [[nodiscard]] std::string uuid_as_str() const;
    [[nodiscard]] bool ready() const;
    [[nodiscard]] State state() const;
    winrt::Windows::Foundation::IAsyncAction query_characteristics();
private:
    explicit CService(GattDeviceService service);
    //winrt::Windows::Foundation::IAsyncAction init();
public:
    std::optional<GattDeviceService> m_Service;
    std::unordered_map<ble::UUID, CCharacteristic, ble::UUID::Hasher> m_Characteristics;
    State m_State;
};
}   // namespace ble