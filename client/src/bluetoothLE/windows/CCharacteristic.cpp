//
// Created by qwerty on 2024-03-01.
//
#include "CCharacteristic.hpp"
// winrt
#include <winrt/Windows.Storage.Streams.h>


#define TO_BOOL(expr) common::enum_to_bool(expr)
namespace
{
constexpr std::size_t INDEX_SEMAPHORE_HAS_SUBSCRIBED = 0u;
constexpr std::size_t INDEX_SEMAPHORE_UNSUBSCRIBE = 1u;
[[nodiscard]] ble::CharacteristicProperties
    operator|(ble::CharacteristicProperties lhs,
              winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties rhs)
{
    return ble::CharacteristicProperties{ std::to_underlying(lhs) | std::to_underlying(rhs) };
}
[[nodiscard]] std::vector<std::string> properties_to_str(ble::CharacteristicProperties properties)
{
    using Properties = ble::CharacteristicProperties;

    std::vector<std::string> props{};
    if (static_cast<bool>(properties & Properties::authenticatedSignedWrites))
    {
        props.emplace_back(ble::characteristic_properties_to_str(properties));
    }
    if (TO_BOOL(properties & Properties::broadcast))
    {
        props.emplace_back(ble::characteristic_properties_to_str(properties));
    }
    if (TO_BOOL(properties & Properties::extendedProperties))
    {
        props.emplace_back(ble::characteristic_properties_to_str(properties));
    }
    if (TO_BOOL(properties & Properties::indicate))
    {
        props.emplace_back(ble::characteristic_properties_to_str(properties));
    }
    if (TO_BOOL(properties & Properties::none))
    {
        props.emplace_back(ble::characteristic_properties_to_str(properties));
    }
    if (TO_BOOL(properties & Properties::notify))
    {
        props.emplace_back(ble::characteristic_properties_to_str(properties));
    }
    if (TO_BOOL(properties & Properties::read))
    {
        props.emplace_back(ble::characteristic_properties_to_str(properties));
    }
    if (TO_BOOL(properties & Properties::reliableWrites))
    {
        props.emplace_back(ble::characteristic_properties_to_str(properties));
    }
    if (TO_BOOL(properties & Properties::writableAuxiliaries))
    {
        props.emplace_back(ble::characteristic_properties_to_str(properties));
    }
    if (TO_BOOL(properties & Properties::write))
    {
        props.emplace_back(ble::characteristic_properties_to_str(properties));
    }
    if (TO_BOOL(properties & Properties::writeWithoutResponse))
    {
        props.emplace_back(ble::characteristic_properties_to_str(properties));
    }


    return props;
}
[[nodiscard]] ble::CharacteristicProperties
    to_props_from_winrt(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties properties)
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    using Properties = ble::CharacteristicProperties;

    Properties props{ 0 };
    if (TO_BOOL(properties & GattCharacteristicProperties::AuthenticatedSignedWrites))
    {
        // cppcheck-suppress badBitmaskCheck
        props = props | GattCharacteristicProperties::AuthenticatedSignedWrites;
    }
    if (TO_BOOL(properties & GattCharacteristicProperties::Broadcast))
    {
        props = props | GattCharacteristicProperties::Broadcast;
    }
    if (TO_BOOL(properties & GattCharacteristicProperties::ExtendedProperties))
    {
        props = props | GattCharacteristicProperties::ExtendedProperties;
    }
    if (TO_BOOL(properties & GattCharacteristicProperties::Indicate))
    {
        props = props | GattCharacteristicProperties::Indicate;
    }
    if (TO_BOOL(properties & GattCharacteristicProperties::None))
    {
        props = props | GattCharacteristicProperties::None;
    }
    if (TO_BOOL(properties & GattCharacteristicProperties::Notify))
    {
        props = props | GattCharacteristicProperties::Notify;
    }
    if (TO_BOOL(properties & GattCharacteristicProperties::Read))
    {
        props = props | GattCharacteristicProperties::Read;
    }
    if (TO_BOOL(properties & GattCharacteristicProperties::ReliableWrites))
    {
        props = props | GattCharacteristicProperties::ReliableWrites;
    }
    if (TO_BOOL(properties & GattCharacteristicProperties::WritableAuxiliaries))
    {
        props = props | GattCharacteristicProperties::WritableAuxiliaries;
    }
    if (TO_BOOL(properties & GattCharacteristicProperties::Write))
    {
        props = props | GattCharacteristicProperties::Write;
    }
    if (TO_BOOL(properties & GattCharacteristicProperties::WriteWithoutResponse))
    {
        props = props | GattCharacteristicProperties::WriteWithoutResponse;
    }

    return props;
}
}    // namespace
namespace ble
{
CCharacteristic::awaitable_make_t CCharacteristic::make(const GattCharacteristic& characteristic)
{
    // Work around because make_shared requires a public constructor
    // But construction of CCharacteristic should go through this factory function
    std::shared_ptr<CCharacteristic> pCharacteristic{ new CCharacteristic{ characteristic } };
    ASSERT(pCharacteristic, "Expected successful construction");

#ifndef NDEBUG
    std::vector<std::string> allProperties = pCharacteristic->properties_as_str();
    std::string line{};
    for (auto&& property : allProperties)
    {
        line += property;
        line += ", ";
    }
    // Remove the two last characters, to make sure the string doesn't end with ', '
    line.pop_back();
    line.pop_back();

    LOG_INFO_FMT("Characteristic UUID: \"{}\""
                 "\nCharacteristic properties: \"{}\""
                 "\nCharacteristic protection level: \"{}\"",
                 winrt::to_string(to_hstring(pCharacteristic->m_Characteristic.Uuid())).c_str(),
                 line.c_str(),
                 prot_level_to_str(pCharacteristic->protection_level()));
#endif

    co_await pCharacteristic->query_descriptors();
    co_return pCharacteristic;
}
CCharacteristic::CCharacteristic(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic characteristic)
    : m_Characteristic{ std::move(characteristic) }
    , m_Descriptors{}
    , m_NotifyEventHandler{}
    , m_Revoker{}
    , m_InFlight{}
{
    std::generate(std::begin(m_InFlight), std::end(m_InFlight), []() { return std::make_unique<std::binary_semaphore>(1); });
}
CCharacteristic::~CCharacteristic()
{
    revoke_value_changed_handler();
}
CCharacteristic::CCharacteristic(CCharacteristic&& other) noexcept
    : m_Characteristic{ std::move(other.m_Characteristic) }
    , m_Descriptors{ std::move(other.m_Descriptors) }
    , m_NotifyEventHandler{ std::move(other.m_NotifyEventHandler) }
    , m_Revoker{ std::move(other.m_Revoker) }
    , m_InFlight{ std::move(other.m_InFlight) } {};
CCharacteristic& CCharacteristic::operator=(CCharacteristic&& other) noexcept
{
    m_Characteristic = std::move(other.m_Characteristic);
    m_Descriptors = std::move(other.m_Descriptors);
    m_NotifyEventHandler = std::move(other.m_NotifyEventHandler);
    m_Revoker = std::move(other.m_Revoker);
    m_InFlight = std::move(other.m_InFlight);


    return *this;
}
CCharacteristic::awaitable_subscription_state_t CCharacteristic::has_subscribed() const
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;

    auto iter = m_Descriptors.find(uuid_descriptor_client_characteristic_configuration_descriptor());
    ASSERT(iter != std::end(m_Descriptors), "has_subscribed called on unsubscribable characteristic!");

    std::binary_semaphore* pInFlight = m_InFlight[INDEX_SEMAPHORE_HAS_SUBSCRIBED].get();
    if (pInFlight->try_acquire())
    {
        auto returnValue = CharacteristicSubscriptionState::notSubscribed;

        std::shared_ptr<CDescriptor> pDescriptor = iter->second;
        ASSERT(pDescriptor, "Expected a valid ptr.");

        std::expected<std::vector<uint8_t>, CommunicationStatus> expectedValue = co_await pDescriptor->read_value();
        if (expectedValue)
        {
            ASSERT_FMT(expectedValue->size() > 1, "Expected at least a uint16 in the Client Configuration Descriptor..");

            // The Client Configuration Descriptor should return two bytes which holds if we are subscribed
            const std::vector<uint8_t>& data = *expectedValue;
            uint16_t value = data[0];
            value = value | data[1];

            if (value & std::to_underlying(GattClientCharacteristicConfigurationDescriptorValue::Notify) ||
                value & std::to_underlying(GattClientCharacteristicConfigurationDescriptorValue::Indicate))
            {
                returnValue = CharacteristicSubscriptionState::subscribed;
            }
        }
        else
        {
            LOG_ERROR_FMT("Failed to query subscription state from Client Configuration Descriptor. Reason: \"{}\"",
                          communication_status_to_str(expectedValue.error()));
        }

        pInFlight->release();
        co_return returnValue;
    }
    else
    {
        co_return CharacteristicSubscriptionState::inFlight;
    }
}
CCharacteristic::awaitable_subscription_state_t CCharacteristic::unsubscribe()
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    ASSERT(m_Characteristic, "Expected a valid characteristic");

    auto returnValue = CharacteristicSubscriptionState::subscribed;

    std::binary_semaphore* pInFlight = m_InFlight[INDEX_SEMAPHORE_UNSUBSCRIBE].get();
    if (pInFlight->try_acquire())
    {
        revoke_value_changed_handler();
        try
        {
            using GattCCCDValue = GattClientCharacteristicConfigurationDescriptorValue;

            auto status = co_await m_Characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattCCCDValue::None);
            if (status == GattCommunicationStatus::Success)
            {
                returnValue = CharacteristicSubscriptionState::notSubscribed;
            }
            else
            {
                LOG_ERROR_FMT("Failed to unsubscribe from Characteristic: \"{}\". Reason: \"{}\"",
                              winrt::to_string(winrt::to_hstring(m_Characteristic.Uuid())).c_str(),
                              communication_status_to_str(communication_status_from_winrt(status)));
            }
        }
        catch (const winrt::hresult_error& err)
        {
            LOG_WARN_FMT("Exception: \"{:X}\" - \"{}\", thrown by WinRT when trying to unsubscribe from Characteristic: \"{}\".",
                         err.code().value,
                         winrt::to_string(winrt::to_hstring(err.message())).c_str(),
                         winrt::to_string(winrt::to_hstring(m_Characteristic.Uuid())).c_str());
        }
        catch (...)
        {
            LOG_ERROR_FMT("Unknown Exception thrown by WinRT when trying to unsubscribe from Characteristic: \"{}\"",
                          winrt::to_string(winrt::to_hstring(m_Characteristic.Uuid())).c_str());
        }

        pInFlight->release();
    }
    else
    {
        returnValue = CharacteristicSubscriptionState::inFlight;
    }

    co_return returnValue;
}
[[nodiscard]] std::string CCharacteristic::uuid_as_str() const
{
    return winrt::to_string(winrt::to_hstring(m_Characteristic.Uuid()));
}
CCharacteristic::awaitable_read_t CCharacteristic::read_value() const
{
    using namespace winrt;
    using namespace Windows::Foundation;
    using namespace Windows::Devices::Bluetooth;
    using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
    using namespace Windows::Storage::Streams;

    try
    {
        GattReadResult result = co_await m_Characteristic.ReadValueAsync(BluetoothCacheMode::Uncached);

        CommunicationStatus status = communication_status_from_winrt(result.Status());
        if (status == CommunicationStatus::success)
        {
            IBuffer buffer = result.Value();

            read_t data{};
            data->resize(buffer.Length());
            std::size_t smallestSize = buffer.Length() <= data->size() ? buffer.Length() : data->size();
            std::memcpy(data->data(), buffer.data(), smallestSize);

            co_return data;
        }
        else
        {
            LOG_WARN_FMT("Failed to read from Characteristic: \"{}\". Reason: \"{}\"",
                         winrt::to_string(winrt::to_hstring(m_Characteristic.Uuid())).c_str(),
                         communication_status_to_str(status));
            co_return std::unexpected(status);
        }
    }
    catch (const winrt::hresult_error& err)
    {
        LOG_WARN_FMT("Exception: \"{:X}\" - \"{}\", thrown by WinRT when trying to read from Characteristic: \"{}\".",
                     err.code().value,
                     winrt::to_string(winrt::to_hstring(err.message())).c_str(),
                     winrt::to_string(winrt::to_hstring(m_Characteristic.Uuid())).c_str());
    }
    catch (...)
    {
        LOG_ERROR_FMT("Unknown Exception thrown by WinRT when trying to read from Characteristic: \"{}\"",
                      winrt::to_string(winrt::to_hstring(m_Characteristic.Uuid())).c_str());
    }

    co_return std::unexpected{ CommunicationStatus::unreachable };
}
CCharacteristic::awaitable_communication_status_t CCharacteristic::write_data(const std::vector<uint8_t>& data) const
{
    co_return co_await write_data(data, GattWriteOption::WriteWithoutResponse);
}
CCharacteristic::awaitable_communication_status_t CCharacteristic::write_data_with_response(const std::vector<uint8_t>& data) const
{
    co_return co_await write_data(data, GattWriteOption::WriteWithResponse);
}
CharacteristicProperties CCharacteristic::properties() const
{
    ASSERT(m_Characteristic, "Expected a valid characteristic.");
    return to_props_from_winrt(m_Characteristic.CharacteristicProperties());
}
std::vector<std::string> CCharacteristic::properties_as_str() const
{
    return properties_to_str(properties());
}
ProtectionLevel CCharacteristic::protection_level() const
{
    ASSERT(m_Characteristic, "Expected a valid characteristic.");
    return protection_level_from_winrt(m_Characteristic.ProtectionLevel());
}
void CCharacteristic::revoke_value_changed_handler()
{
    if (m_Revoker)
    {
        m_Revoker.revoke();
    }
}
auto CCharacteristic::value_changed_handler()
{
    return [wpSelf = weak_from_this()](GattCharacteristic characteristic, const GattValueChangedEventArgs& args)
    {
        std::shared_ptr<CCharacteristic> pSelf = wpSelf.lock();
        if (pSelf)
        {
            winrt::Windows::Storage::Streams::IBuffer buffer = args.CharacteristicValue();
            ASSERT(pSelf->m_NotifyEventHandler, "Expected a event handler");
            pSelf->m_NotifyEventHandler(std::span<uint8_t>{ buffer.data(), buffer.Length() });
        }
    };
}
void CCharacteristic::register_value_changed_handler()
{
    // Unusure if this callback is characteristic specific or one big callback for ALL subscribed characteristics
    // - Its not a problem for now since there exist only a single subscribable characteristic..
    m_Revoker = m_Characteristic.ValueChanged(winrt::auto_revoke, value_changed_handler());
}
CCharacteristic::awaitable_communication_status_t CCharacteristic::write_data(const std::vector<uint8_t>& data,
                                                                              GattWriteOption option) const
{
    using Buffer = winrt::Windows::Storage::Streams::Buffer;


    Buffer buffer{ static_cast<uint32_t>(data.size()) };
    buffer.Length(buffer.Capacity());
    std::size_t smallestSize = buffer.Length() <= data.size() ? buffer.Length() : data.size();
    std::memcpy(buffer.data(), data.data(), smallestSize);

    try
    {
        co_return communication_status_from_winrt(co_await m_Characteristic.WriteValueAsync(buffer, option));
    }
    catch (const winrt::hresult_error& err)
    {
        LOG_WARN_FMT("Exception: \"{:X}\" - \"{}\", thrown by WinRT when trying to write to Characteristic: \"{}\".",
                     err.code().value,
                     winrt::to_string(winrt::to_hstring(err.message())).c_str(),
                     winrt::to_string(winrt::to_hstring(m_Characteristic.Uuid())).c_str());
    }
    catch (...)
    {
        LOG_ERROR_FMT("Unknown Exception thrown by WinRT when trying to write to Characteristic: \"{}\"",
                      winrt::to_string(winrt::to_hstring(m_Characteristic.Uuid())).c_str());
    }

    co_return CommunicationStatus::unreachable;
}
winrt::Windows::Foundation::IAsyncAction CCharacteristic::query_descriptors()
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    using namespace winrt::Windows::Foundation::Collections;


    m_Descriptors.clear();
    try
    {
        GattDescriptorsResult result = co_await m_Characteristic.GetDescriptorsAsync();
        if (result.Status() == GattCommunicationStatus::Success)
        {
            IVectorView<GattDescriptor> descriptors = result.Descriptors();
            m_Descriptors.reserve(descriptors.Size());

            for (auto&& descriptor : descriptors)
            {
                auto [iter, emplaced] =
                    m_Descriptors.try_emplace(make_uuid(descriptor.Uuid()), co_await make_descriptor<CDescriptor>(descriptor));
                if (!emplaced)
                {
                    LOG_ERROR_FMT("Failed to emplace descriptor with UUID: \"{}\"", uuid_as_str());
                }
            }
        }
        else
        {
            LOG_ERROR_FMT("Communication error: \"{}\" when trying to query Descriptors from Characteristic with UUID: \"{}\"",
                          communication_status_to_str(communication_status_from_winrt(result.Status())),
                          uuid_as_str());
        }
    }
    catch (const winrt::hresult_error& err)
    {
        LOG_WARN_FMT("Exception: \"{:X}\" - \"{}\", thrown by WinRT when trying to query descriptors from Characteristic: \"{}\".",
                     err.code().value,
                     winrt::to_string(winrt::to_hstring(err.message())).c_str(),
                     winrt::to_string(winrt::to_hstring(m_Characteristic.Uuid())).c_str());
    }
    catch (...)
    {
        LOG_ERROR_FMT("Unknown Exception thrown by WinRT when trying to write to query descriptors from Characteristic: \"{}\"",
                      winrt::to_string(winrt::to_hstring(m_Characteristic.Uuid())).c_str());
    }
}
}    // namespace ble
