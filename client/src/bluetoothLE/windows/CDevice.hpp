//
// Created by qwerty on 2024-02-23.
//

#pragma once
#include "CService.hpp"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Foundation.Collections.h>


namespace ble::win
{
class CDevice
{
public:
    enum class State : uint32_t
    {
        uninitialized,
        invalidAddress,
        queryingServices,
        ready
    };
public:
    [[nodiscard]] static CDevice make_device(uint64_t address);
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
private:
    CDevice() = default;
    winrt::Windows::Foundation::IAsyncAction init(uint64_t address);
public:
    std::optional<winrt::Windows::Devices::Bluetooth::BluetoothLEDevice> m_Device;
    State m_State;
    std::unordered_map<ble::UUID, CService, ble::UUID::Hasher> m_Services;
};
}   // namespace ble::win