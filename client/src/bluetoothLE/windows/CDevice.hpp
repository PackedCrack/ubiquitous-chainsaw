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
    using service_container_t = std::unordered_map<UUID, CService, UUID::Hasher>;
private:
    using BluetoothLEDevice = winrt::Windows::Devices::Bluetooth::BluetoothLEDevice;
    using IAsyncAction = winrt::Windows::Foundation::IAsyncAction;
public:
    [[nodiscard]] static awaitable_t make(uint64_t address);
    CDevice() = default;
    ~CDevice() = default;
    CDevice(const CDevice& other) = default;
    CDevice(CDevice&& other) noexcept = default;
    CDevice& operator=(const CDevice& other) = default;
    CDevice& operator=(CDevice&& other) = default;
public:
    [[nodiscard]] uint64_t address() const;
    [[nodiscard]] std::string address_as_str() const;
    [[nodiscard]] const service_container_t& services() const;
    [[nodiscard]] std::optional<const CService*> service(const UUID& uuid) const;
private:
    IAsyncAction query_services();
private:
    std::shared_ptr<BluetoothLEDevice> m_pDevice;
    service_container_t m_Services;
};
}   // namespace ble