//
// Created by qwerty on 2024-02-23.
//

#pragma once
#include "../Service.hpp"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Foundation.Collections.h>


namespace ble
{
class CDevice
{
public:
    using awaitable_t = concurrency::task<CDevice>;
    enum class State : uint32_t
    {
        uninitialized,
        invalidAddress,
        queryingServices,
        ready
    };
public:
    CDevice() = default;
    [[nodiscard]] static awaitable_t make(uint64_t address);
    ~CDevice() = default;
    CDevice(const CDevice& other) = default;
    CDevice(CDevice&& other) = default;
    CDevice& operator=(const CDevice& other) = default;
    CDevice& operator=(CDevice&& other) = default;
public:
    [[nodiscard]] uint64_t address() const;
    [[nodiscard]] std::string address_as_str() const;
    [[nodiscard]] bool ready() const;
    [[nodiscard]] State state() const;
    [[nodiscard]] const std::unordered_map<ble::UUID, CService, ble::UUID::Hasher>& services() const;
    winrt::Windows::Foundation::IAsyncAction query_services();
public:
    std::optional<winrt::Windows::Devices::Bluetooth::BluetoothLEDevice> m_Device;
    State m_State = State::uninitialized;
    std::unordered_map<ble::UUID, CService, ble::UUID::Hasher> m_Services;
};


template<typename T>
class DeviceWrapper
{
public:
private:
    T m_Device;
};

}   // namespace ble