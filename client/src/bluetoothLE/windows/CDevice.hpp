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
    enum class Error
    {
        invalidAddress
    };
public:
    using make_t = std::expected<CDevice, Error>;
    using awaitable_t = concurrency::task<make_t>;
    using service_container_t = std::unordered_map<UUID, CService, UUID::Hasher>;
private:
    using BluetoothLEDevice = winrt::Windows::Devices::Bluetooth::BluetoothLEDevice;
    using IAsyncAction = winrt::Windows::Foundation::IAsyncAction;
public:
    [[nodiscard]] static awaitable_t make(uint64_t address);
    CDevice() = default;
    ~CDevice() = default;
    CDevice(const CDevice& other);
    CDevice(CDevice&& other) noexcept;
    CDevice& operator=(const CDevice& other);
    CDevice& operator=(CDevice&& other) noexcept;
private:
    void copy(const CDevice& other);
    void move(CDevice& other);
public:
    template<typename invokable_t>
    requires std::invocable<invokable_t, ConnectionStatus>
    void set_connection_changed_cb(invokable_t&& cb)
    {
        m_ConnectionChanged = std::forward<invokable_t>(cb);
        m_pDevice->ConnectionStatusChanged(
            [this](const winrt::Windows::Devices::Bluetooth::BluetoothLEDevice& device,
                   [[maybe_unused]] const winrt::Windows::Foundation::IInspectable& inspectable)
            {
                using Status = winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus;

                Status status = device.ConnectionStatus();
                ConnectionStatus conStatus{};
                if (status == Status::Connected)
                {
                    conStatus = ConnectionStatus::connected;
                }
                else if (status == Status::Disconnected)
                {
                    conStatus = ConnectionStatus::disconnected;
                }
                else
                {
                    assert(false);
                }

                this->m_ConnectionChanged(conStatus);
            });
    }
    [[nodiscard]] bool connected() const;
    [[nodiscard]] uint64_t address() const;
    [[nodiscard]] std::string address_as_str() const;
    [[nodiscard]] const service_container_t& services() const;
    [[nodiscard]] std::optional<const CService*> service(const UUID& uuid) const;
private:
    IAsyncAction query_services();
private:
    std::shared_ptr<BluetoothLEDevice> m_pDevice;
    service_container_t m_Services;
    std::function<void(ConnectionStatus)> m_ConnectionChanged;
};
}    // namespace ble
