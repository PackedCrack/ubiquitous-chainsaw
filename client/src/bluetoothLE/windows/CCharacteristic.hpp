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
#include <semaphore>
// clang-format off


// clang-format on
namespace ble
{
class CCharacteristic : public std::enable_shared_from_this<CCharacteristic>
{
public:
    // We return shared ptrs because concurrency::task<T> requires that T has a copy constructor
    using awaitable_make_t = concurrency::task<std::shared_ptr<CCharacteristic>>;
    using read_t = std::expected<std::vector<uint8_t>, CommunicationStatus>;
    using awaitable_read_t = concurrency::task<read_t>;
    using awaitable_communication_status_t = concurrency::task<CommunicationStatus>;
    using awaitable_subscription_state_t = concurrency::task<CharacteristicSubscriptionState>;
private:
    using GattCharacteristic = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic;
    using GattWriteOption = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattWriteOption;
    using GattValueChangedEventArgs = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattValueChangedEventArgs;
public:
    [[nodiscard]] static awaitable_make_t make(const GattCharacteristic& characteristic);
    //CCharacteristic() = default;
    ~CCharacteristic();
    CCharacteristic(const CCharacteristic& other) = delete;
    CCharacteristic(CCharacteristic&& other) noexcept;
    CCharacteristic& operator=(const CCharacteristic& other) = delete;
    CCharacteristic& operator=(CCharacteristic&& other) noexcept;
private:
    explicit CCharacteristic(GattCharacteristic characteristic);
public:
    template<typename invokable_t>
    requires std::invocable<invokable_t, std::span<const uint8_t>>
    [[nodiscard]] awaitable_subscription_state_t subscribe_to_notify(invokable_t&& cb)
    {
        using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
        ASSERT(m_Characteristic, "Expected a valid characteristic");

        // Placing this here because we don't have access to the constants in the translation unit since this is a template function
        static constexpr std::size_t INDEX_SEMAPHORE_SUBSCRIBE = 2u;
        
        std::binary_semaphore* pInFlight = m_InFlight[INDEX_SEMAPHORE_SUBSCRIBE].get();
        if (pInFlight->try_acquire())
        {
            m_NotifyEventHandler = std::forward<invokable_t>(cb);
            register_value_changed_handler();

            try
            {
                using GattCCCDValue = GattClientCharacteristicConfigurationDescriptorValue;

                GattCommunicationStatus status = co_await m_Characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattCCCDValue::Notify);
                if (status == GattCommunicationStatus::Success)
                {
                    co_return CharacteristicSubscriptionState::subscribed;
                }
                else
                {
                    LOG_ERROR_FMT("Failed to subscribe to Characteristic: \"{}\". Reason: \"{}\"",
                        winrt::to_string(winrt::to_hstring(m_Characteristic.Uuid())).c_str(),
                        communication_status_to_str(communication_status_from_winrt(status)));
                }
            }
            catch (const winrt::hresult_error& err)
            {
                LOG_WARN_FMT("Exception: \"{:X}\" - \"{}\", thrown by WinRT when trying to subscribe to Characteristic: \"{}\".",
                    err.code().value,
                    winrt::to_string(winrt::to_hstring(err.message())).c_str(),
                    winrt::to_string(winrt::to_hstring(m_Characteristic.Uuid())).c_str());
            }
            catch (...)
            {
                LOG_ERROR_FMT("Unknown Exception caught when trying to subscribe to Characteristic: \"{}\".",
                    winrt::to_string(to_hstring(m_Characteristic.Uuid())).c_str());
            }

            co_return CharacteristicSubscriptionState::notSubscribed;
        }
        else
        {
            co_return CharacteristicSubscriptionState::inFlight;
        }
    }
    [[nodiscard]] awaitable_subscription_state_t has_subscribed() const;
    [[nodiscard]] awaitable_subscription_state_t unsubscribe();
    [[nodiscard]] std::string uuid_as_str() const;
    [[nodiscard]] awaitable_read_t read_value() const;
    [[nodiscard]] awaitable_communication_status_t write_data(const std::vector<uint8_t>& data) const;
    [[nodiscard]] awaitable_communication_status_t write_data_with_response(const std::vector<uint8_t>& data) const;
    [[nodiscard]] CharacteristicProperties properties() const;
    [[nodiscard]] std::vector<std::string> properties_as_str() const;
    [[nodiscard]] ProtectionLevel protection_level() const;
private:
    void revoke_value_changed_handler();
    void register_value_changed_handler();
    [[nodiscard]] auto value_changed_handler();
    [[nodiscard]] awaitable_communication_status_t write_data(const std::vector<uint8_t>& data, GattWriteOption option) const;
    winrt::Windows::Foundation::IAsyncAction query_descriptors();
private:
    GattCharacteristic m_Characteristic;
    std::unordered_map<ble::UUID, std::shared_ptr<CDescriptor>, ble::UUID::Hasher> m_Descriptors;
    std::function<void(std::span<const uint8_t>)> m_NotifyEventHandler;
    GattCharacteristic::ValueChanged_revoker m_Revoker;
    std::array<std::unique_ptr<std::binary_semaphore>, 3u> m_InFlight;
};
}    // namespace ble
