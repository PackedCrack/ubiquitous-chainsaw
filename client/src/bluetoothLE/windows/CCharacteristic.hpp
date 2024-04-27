//
// Created by qwerty on 2024-03-01.
//
#pragma once
#include "common/ble_services.hpp"
#include "../../client_defines.hpp"
#include "../Descriptor.hpp"
#include "win_ble_common.hpp"
// winrt
#pragma warning(push)
#pragma warning(disable: 4'265)    // missing virtual destructor - wtf microsfot?
#include <winrt/Windows.Foundation.h>
#pragma warning(pop)
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
// clang-format off


// clang-format on
namespace ble
{
class CCharacteristic
{
public:
    using awaitable_t = concurrency::task<CCharacteristic>;
    using read_t = std::expected<std::vector<uint8_t>, CommunicationStatus>;
    using awaitable_read_t = concurrency::task<read_t>;
    using awaitable_write_t = concurrency::task<CommunicationStatus>;
    using awaitable_subscribe_t = concurrency::task<CommunicationStatus>;
private:
    using GattCharacteristic = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic;
    using GattWriteOption = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattWriteOption;
    using GattValueChangedEventArgs = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattValueChangedEventArgs;
public:
    [[nodiscard]] static awaitable_t make(const GattCharacteristic& characteristic);
    CCharacteristic() = default;
    ~CCharacteristic();
    CCharacteristic(const CCharacteristic& other);
    CCharacteristic(CCharacteristic&& other) noexcept;
    CCharacteristic& operator=(const CCharacteristic& other);
    CCharacteristic& operator=(CCharacteristic&& other) noexcept;
private:
    explicit CCharacteristic(GattCharacteristic characteristic);
public:
    template<typename invokable_t>
    requires std::invocable<invokable_t, std::span<const uint8_t>>
    [[nodiscard]] awaitable_subscribe_t subscribe_to_notify(invokable_t&& cb)
    {
        using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;

        m_NotifyEventHandler = std::forward<invokable_t>(cb);
        ASSERT(m_pCharacteristic, "Expected a valid characteristic");
        m_Revoker = m_pCharacteristic->ValueChanged(winrt::auto_revoke, value_changed_handler());

        co_return winrt_status_to_communication_status(co_await m_pCharacteristic->WriteClientCharacteristicConfigurationDescriptorAsync(
            GattClientCharacteristicConfigurationDescriptorValue::Notify));
    }
    [[nodiscard]] std::string uuid_as_str() const;
    [[nodiscard]] awaitable_read_t read_value() const;
    [[nodiscard]] awaitable_write_t write_data(const std::vector<uint8_t>& data) const;
    [[nodiscard]] awaitable_write_t write_data_with_response(const std::vector<uint8_t>& data) const;
    [[nodiscard]] CharacteristicProperties properties() const;
    [[nodiscard]] std::vector<std::string> properties_as_str() const;
    [[nodiscard]] ProtectionLevel protection_level() const;
private:
    void revoke_value_changed_handler();
    void register_value_changed_handler();
    void refresh_value_changed_handler();
    [[nodiscard]] std::function<void(GattCharacteristic characteristic, const GattValueChangedEventArgs& args)> value_changed_handler();
    [[nodiscard]] awaitable_write_t write_data(const std::vector<uint8_t>& data, GattWriteOption option) const;
    winrt::Windows::Foundation::IAsyncAction query_descriptors();
private:
    std::shared_ptr<GattCharacteristic> m_pCharacteristic;
    std::unordered_map<ble::UUID, CDescriptor, ble::UUID::Hasher> m_Descriptors;
    std::function<void(std::span<const uint8_t> packetView)> m_NotifyEventHandler;
    GattCharacteristic::ValueChanged_revoker m_Revoker;
};
}    // namespace ble
