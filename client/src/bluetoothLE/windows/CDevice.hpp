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
class CDevice : public std::enable_shared_from_this<CDevice>
{
public:
    enum class Error
    {
        invalidAddress,
        failedToQueryServices,
        constructorError
    };
public:
    using make_t = std::expected<std::shared_ptr<CDevice>, Error>;
    using awaitable_make_t = concurrency::task<make_t>;
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

        try
        {
            // Work around because make_shared requires a public constructor
            // But construction of CDevice should go through this factory function
            std::expected<std::shared_ptr<CDevice>, CDevice::Error> expected{ new CDevice{ std::forward<invokable_t>(cb) } };

            // For clarity
            CDevice* pDevice = expected.value().get();
            BluetoothLEDevice& device = pDevice->m_Device;

            device = co_await BluetoothLEDevice::FromBluetoothAddressAsync(address);
            /*
            * The returned BluetoothLEDevice is set to null if
            * FromBluetoothAddressAsync can't find the device identified by bluetoothAddress.
            * */
            if (device)
            {
                bool success = co_await pDevice->query_services(address);
                if (success)
                {
                    pDevice->register_connection_changed_handler();
                    co_return expected;
                }
                else
                {
                    co_return std::unexpected{ Error::failedToQueryServices };
                }
            }
            else
            {
                LOG_WARN_FMT("Failed to instantiate CDevice: Could not find a peripheral with address: \"{}\"",
                             ble::hex_addr_to_str(address));
                co_return std::unexpected{ Error::invalidAddress };
            }
        }
        catch (const winrt::hresult_error& err)
        {
            LOG_WARN_FMT("Exception: \"{:X}\" - \"{}\", thrown by WinRT when trying to construct CDevice from address: \"{}\".",
                         err.code().value,
                         winrt::to_string(winrt::to_hstring(err.message())).c_str(),
                         hex_addr_to_str(address).c_str());
        }
        catch (...)
        {
            LOG_ERROR_FMT("Unknown Exception thrown by WinRT when trying to construct CDevice from address: \"{}\"",
                          hex_addr_to_str(address).c_str());
        }

        co_return std::unexpected{ Error::constructorError };
    }
    ~CDevice();
    // Cant copy because we must register a callback with winrt - there can only be one
    CDevice(const CDevice& other) = delete;
    CDevice(CDevice&& other) noexcept;
    CDevice& operator=(const CDevice& other) = delete;
    CDevice& operator=(CDevice&& other) noexcept;
private:
    template<typename invokable_t>
    requires std::invocable<invokable_t, ConnectionStatus>
    CDevice(invokable_t&& cb)
        : m_Device{ nullptr }
        , m_Services{}
        , m_ConnectionChanged{ std::forward<invokable_t>(cb) }
        , m_Revoker{ m_Device.ConnectionStatusChanged(winrt::auto_revoke, connection_changed_handler()) }
    {}
public:
    [[nodiscard]] static std::string_view error_to_str(Error err);
    [[nodiscard]] bool connected() const;
    [[nodiscard]] uint64_t address() const;
    [[nodiscard]] std::string address_as_str() const;
    [[nodiscard]] const service_container_t& services() const;
    [[nodiscard]] std::optional<std::weak_ptr<CService>> service(const UUID& uuid) const;
private:
    [[nodiscard]] std::function<void(const BluetoothLEDevice& device, const IInspectable& inspectable)> connection_changed_handler();
    [[nodiscard]] concurrency::task<bool> query_services(uint64_t address);
    void revoke_connection_changed_handler();
    void register_connection_changed_handler();
private:
    BluetoothLEDevice m_Device;
    service_container_t m_Services;
    std::function<void(ConnectionStatus)> m_ConnectionChanged;
    BluetoothLEDevice::ConnectionStatusChanged_revoker m_Revoker;
};
}    // namespace ble
