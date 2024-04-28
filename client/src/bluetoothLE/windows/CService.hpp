//
// Created by qwerty on 2024-03-01.
//
#pragma once
#include "../../system/System.hpp"
#include "../Characteristic.hpp"
// clang-format off


// clang-format on
namespace ble
{
class CService
{
public:
    using awaitable_make_t = concurrency::task<CService>;
    using awaitable_subscribe_t = concurrency::task<bool>;
private:
    using GattDeviceService = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService;
public:
    [[nodiscard]] static awaitable_make_t make(const GattDeviceService& service);
    CService() = default;
    ~CService();
    CService(const CService& other) = default;
    CService(CService&& other) noexcept = default;
    CService& operator=(const CService& other) = default;
    CService& operator=(CService&& other) = default;
private:
    explicit CService(GattDeviceService service);
public:
    [[nodiscard]] std::optional<std::weak_ptr<CCharacteristic>> characteristic(const UUID& uuid) const;
    [[nodiscard]] std::string uuid_as_str() const;
    template<typename invokable_t>
    requires std::invocable<invokable_t, std::span<const uint8_t>>
    [[nodiscard]] awaitable_subscribe_t subscribe_to_characteristic(const UUID& characteristic, invokable_t&& cb)
    {
        auto iter = m_Characteristics.find(characteristic);
        if (iter == std::end(m_Characteristics))
        {
            co_return false;
        }


        uint16_t uniqueValue = characteristic.data[2] << 8;
        uniqueValue = uniqueValue | characteristic.data[3];
        UNHANDLED_CASE_PROTECTION_ON
        switch (co_await iter->second->subscribe_to_notify(std::forward<invokable_t>(cb)))
        {
        case CommunicationStatus::success:
        {
            co_return true;
        }
        case CommunicationStatus::unreachable:
        {
            LOG_WARN_FMT("Failed to subscribe to \"{:X}\". Device was unreachable.", uniqueValue);
            break;
        }
        case CommunicationStatus::accessDenied:
        {
            LOG_WARN_FMT("Failed to subscribe to \"{:X}\". Access was denied.", uniqueValue);
            break;
        }
        case CommunicationStatus::protocolError:
        {
            LOG_WARN_FMT("Failed to subscribe to \"{:X}\". There was a protocol error.", uniqueValue);
            break;
        }
        }
        UNHANDLED_CASE_PROTECTION_OFF

        co_return false;
    }
    sys::fire_and_forget_t unsubscribe_from_characteristic(const UUID& characteristic);
private:
    winrt::Windows::Foundation::IAsyncAction query_characteristics();
private:
    std::shared_ptr<GattDeviceService> m_pService;
    std::unordered_map<ble::UUID, std::shared_ptr<CCharacteristic>, ble::UUID::Hasher> m_Characteristics;
};
}    // namespace ble
