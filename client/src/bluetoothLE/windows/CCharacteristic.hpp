//
// Created by qwerty on 2024-03-01.
//
#pragma once
#include "common/ble_services.hpp"
#include "../../client_defines.hpp"
#include "../Descriptor.hpp"
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
    enum class Properties : uint32_t
    {
        none = 0,
        broadcast = 0x1,
        read = 0x2,
        writeWithoutResponse = 0x4,
        write = 0x8,
        notify = 0x10,
        indicate = 0x20,
        authenticatedSignedWrites = 0x40,
        extendedProperties = 0x80,
        reliableWrites = 0x1'00,
        writableAuxiliaries = 0x2'00,
    };
    [[nodiscard]] friend constexpr Properties operator&(Properties lhs, Properties rhs)
    {
        return Properties{ std::to_underlying(lhs) & std::to_underlying(rhs) };
    }
    [[nodiscard]] friend constexpr Properties operator|(Properties lhs, Properties rhs)
    {
        return Properties{ std::to_underlying(lhs) | std::to_underlying(rhs) };
    }
    [[nodiscard]] friend constexpr Properties operator^(Properties lhs, Properties rhs)
    {
        return Properties{ std::to_underlying(lhs) ^ std::to_underlying(rhs) };
    }
    [[nodiscard]] friend constexpr Properties operator~(Properties prop) { return Properties{ std::to_underlying(prop) }; }
private:
    using GattCharacteristic = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic;
    using GattWriteOption = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattWriteOption;
    using GattValueChangedEventArgs = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattValueChangedEventArgs;
public:
    [[nodiscard]] static awaitable_t make(const GattCharacteristic& characteristic);
    CCharacteristic() = default;
    virtual ~CCharacteristic();
    CCharacteristic(const CCharacteristic& other);
    CCharacteristic(CCharacteristic&& other) noexcept;
    CCharacteristic& operator=(const CCharacteristic& other);
    CCharacteristic& operator=(CCharacteristic&& other) noexcept;
private:
    explicit CCharacteristic(GattCharacteristic characteristic);
public:
    template<typename invokable_t>
    requires std::invocable<invokable_t, std::vector<uint8_t>&&>
    void register_notify_event_handler(invokable_t&& invokable)
    {
        m_NotifyEventHandler = std::forward<invokable_t>(invokable);
        update_value_changed_event();
    }
    [[nodiscard]] std::string uuid_as_str() const;
    [[nodiscard]] awaitable_read_t read_value() const;
    [[nodiscard]] awaitable_write_t write_data(const std::vector<uint8_t>& data) const;
    [[nodiscard]] awaitable_write_t write_data_with_response(const std::vector<uint8_t>& data) const;
    [[nodiscard]] awaitable_subscribe_t subscribe_to_notify() const;
private:
    void update_value_changed_event();
    void deregister_current_value_changed_event();
    [[nodiscard]] awaitable_write_t write_data(const std::vector<uint8_t>& data, GattWriteOption option) const;
    winrt::Windows::Foundation::IAsyncAction query_descriptors();
private:
    std::shared_ptr<GattCharacteristic> m_pCharacteristic;
    std::unordered_map<ble::UUID, CDescriptor, ble::UUID::Hasher> m_Descriptors;
    ProtectionLevel m_ProtLevel = ProtectionLevel::plain;
    Properties m_Properties = Properties::none;
    std::optional<winrt::event_token> m_Cookie;
    std::function<void(std::vector<uint8_t>&& buffer)> m_NotifyEventHandler;
};
}    // namespace ble
