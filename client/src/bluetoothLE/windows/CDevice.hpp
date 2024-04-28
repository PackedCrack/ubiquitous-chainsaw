//
// Created by qwerty on 2024-02-23.
//
#pragma once
#include "../Service.hpp"
// winrt
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Foundation.Collections.h>
// clang-format off


// clang-format on
namespace ble
{
class CDevice
{
public:
    enum class Error
    {
        invalidAddress
    };
public:
    using make_t = std::expected<CDevice, Error>;
    using awaitable_make_t = concurrency::task<make_t>;
    using awaitable_subscribe_t = concurrency::task<bool>;
    using service_container_t = std::unordered_map<UUID, std::shared_ptr<CService>, UUID::Hasher>;
private:
    using BluetoothLEDevice = winrt::Windows::Devices::Bluetooth::BluetoothLEDevice;
    using IAsyncAction = winrt::Windows::Foundation::IAsyncAction;
    using IInspectable = winrt::Windows::Foundation::IInspectable;
public:
    //[[nodiscard]] static awaitable_t make(uint64_t address);
    template<typename invokable_t>
    requires std::invocable<invokable_t, ConnectionStatus>
    [[nodiscard]] static awaitable_make_t make(uint64_t address, invokable_t&& cb)
    {
        using namespace winrt::Windows::Devices::Bluetooth;

        std::expected<CDevice, CDevice::Error> expected{};
        expected->m_pDevice = std::make_shared<BluetoothLEDevice>(co_await BluetoothLEDevice::FromBluetoothAddressAsync(address));
        /*
        * The returned BluetoothLEDevice is set to null if
        * FromBluetoothAddressAsync can't find the device identified by bluetoothAddress.
        * */
        if (*(expected->m_pDevice))
        {
            expected->m_ConnectionChanged = std::forward<invokable_t>(cb);
            co_await expected->query_services();
        }
        else
        {
            LOG_WARN_FMT("Failed to instantiate CDevice: Could not find a peripheral with address: \"{}\"", ble::hex_addr_to_str(address));
            co_return std::unexpected(Error::invalidAddress);
        }

        co_return expected;
    }
    CDevice() = default;
    ~CDevice();
    CDevice(const CDevice& other);
    CDevice(CDevice&& other) noexcept;
    CDevice& operator=(const CDevice& other);
    CDevice& operator=(CDevice&& other) noexcept;
public:
    [[nodiscard]] bool connected() const;
    [[nodiscard]] uint64_t address() const;
    [[nodiscard]] std::string address_as_str() const;
    [[nodiscard]] const service_container_t& services() const;
    [[nodiscard]] std::optional<std::weak_ptr<CService>> service(const UUID& uuid) const;
    template<typename invokable_t>
    requires std::invocable<invokable_t, std::span<const uint8_t>>
    [[nodiscard]] awaitable_subscribe_t
        subscribe_to_characteristic(const ble::UUID& service, const ble::UUID& characteristic, invokable_t&& cb)
    {
        auto iter = m_Services.find(service);
        if (iter == std::end(m_Services))
        {
            co_return false;
        }

        co_return co_await iter->second->subscribe_to_characteristic(characteristic, std::forward<invokable_t>(cb));
    }
    void unsubscribe_from_characteristic(const ble::UUID& service, const ble::UUID& characteristic);
private:
    void revoke_connection_changed_handler();
    void register_connection_changed_handler();
    void refresh_connection_changed_handler();
    [[nodiscard]] std::function<void(const BluetoothLEDevice& device, [[maybe_unused]] const IInspectable& inspectable)>
        connection_changed_handler();
    IAsyncAction query_services();
private:
    std::shared_ptr<BluetoothLEDevice> m_pDevice;
    service_container_t m_Services;
    std::function<void(ConnectionStatus)> m_ConnectionChanged;
    BluetoothLEDevice::ConnectionStatusChanged_revoker m_Revoker;
};
}    // namespace ble
