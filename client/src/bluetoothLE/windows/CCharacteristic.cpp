//
// Created by qwerty on 2024-03-01.
//
#include "CCharacteristic.hpp"
// winrt
#include <winrt/Windows.Storage.Streams.h>


#define TO_BOOL(expr) common::enum_to_bool(expr)
namespace
{
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
CCharacteristic::awaitable_t CCharacteristic::make(const GattCharacteristic& characteristic)
{
    CCharacteristic charac{ characteristic };

#ifndef NDEBUG
    std::vector<std::string> allProperties = charac.properties_as_str();
    std::printf("\nCharacteristic UUID: %ws", to_hstring(charac.m_pCharacteristic->Uuid()).data());
    std::printf("\nCharacteristic properties: ");
    for (auto&& property : allProperties)
    {
        std::printf("%s, ", property.c_str());
    }
    std::printf("\n%s", std::format("Characteristic protection level: \"{}\"", prot_level_to_str(charac.protection_level())).c_str());
#endif

    co_await charac.query_descriptors();
    co_return charac;
}
CCharacteristic::CCharacteristic(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic characteristic)
    : m_pCharacteristic{ std::make_shared<GattCharacteristic>(std::move(characteristic)) }
    , m_Descriptors{}
    , m_NotifyEventHandler{}
    , m_Revoker{}
{}
CCharacteristic::~CCharacteristic()
{
    revoke_value_changed_handler();
}
CCharacteristic::CCharacteristic(const CCharacteristic& other)
    : m_pCharacteristic{ other.m_pCharacteristic }
    , m_Descriptors{ other.m_Descriptors }
    , m_NotifyEventHandler{ other.m_NotifyEventHandler }
    , m_Revoker{}
{
    refresh_value_changed_handler();
}
CCharacteristic::CCharacteristic(CCharacteristic&& other) noexcept
    : m_pCharacteristic{ std::move(other.m_pCharacteristic) }
    , m_Descriptors{ std::move(other.m_Descriptors) }
    , m_NotifyEventHandler{ std::move(other.m_NotifyEventHandler) }
    , m_Revoker{ std::move(other.m_Revoker) }
{
    refresh_value_changed_handler();
};
CCharacteristic& CCharacteristic::operator=(const CCharacteristic& other)
{
    if (this != &other)
    {
        m_pCharacteristic = other.m_pCharacteristic;
        m_Descriptors = other.m_Descriptors;
        m_NotifyEventHandler = other.m_NotifyEventHandler;
        m_Revoker = GattCharacteristic::ValueChanged_revoker{};
        refresh_value_changed_handler();
    }

    return *this;
}
CCharacteristic& CCharacteristic::operator=(CCharacteristic&& other) noexcept
{
    m_pCharacteristic = std::move(other.m_pCharacteristic);
    m_Descriptors = std::move(other.m_Descriptors);
    m_NotifyEventHandler = std::move(other.m_NotifyEventHandler);
    m_Revoker = std::move(other.m_Revoker);
    refresh_value_changed_handler();

    return *this;
}
CCharacteristic::awaitable_bool_t CCharacteristic::has_subscribed() const
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;

    auto iter = m_Descriptors.find(uuid_descriptor_client_characteristic_configuration_descriptor());
    ASSERT(iter != std::end(m_Descriptors), "has_subscribed called on unsubscribable characteristic!");

    std::expected<std::vector<uint8_t>, CommunicationStatus> expectedValue = co_await iter->second->read_value();
    if (expectedValue)
    {
        ASSERT_FMT(expectedValue->size() > 1, "Expected at least a uint16 in the Client Configuration Descriptor..");

        const std::vector<uint8_t>& data = *expectedValue;
        uint16_t value = data[0];
        value = value | data[1];

        if (value & std::to_underlying(GattClientCharacteristicConfigurationDescriptorValue::Notify))
        {
            co_return true;
        }
        if (value & std::to_underlying(GattClientCharacteristicConfigurationDescriptorValue::Indicate))
        {
            co_return true;
        }
    }
    else
    {
        LOG_ERROR_FMT("Could not read Client Configuration Descriptor. Reason: \"{}\"", communication_status_to_str(expectedValue.error()));
    }

    co_return false;
}
CCharacteristic::awaitable_unsubscribe_t CCharacteristic::unsubscribe()
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    ASSERT(m_pCharacteristic, "Expected a valid characteristic");

    revoke_value_changed_handler();
    co_return communication_status_from_winrt(co_await m_pCharacteristic->WriteClientCharacteristicConfigurationDescriptorAsync(
        GattClientCharacteristicConfigurationDescriptorValue::None));
}
[[nodiscard]] std::string CCharacteristic::uuid_as_str() const
{
    return winrt::to_string(winrt::to_hstring(m_pCharacteristic->Uuid()));
}
CCharacteristic::awaitable_read_t CCharacteristic::read_value() const
{
    using namespace winrt;
    using namespace Windows::Foundation;
    using namespace Windows::Devices::Bluetooth;
    using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
    using namespace Windows::Storage::Streams;

    GattReadResult result = co_await m_pCharacteristic->ReadValueAsync(BluetoothCacheMode::Uncached);

    CommunicationStatus status = communication_status_from_winrt(result.Status());
    if (status == CommunicationStatus::success)
    {
        IBuffer buffer = result.Value();

        read_t data{};
        data->resize(buffer.Length());
        size_t smallestSize = buffer.Length() <= data->size() ? buffer.Length() : data->size();
        std::memcpy(data->data(), buffer.data(), smallestSize);

        co_return data;
    }
    else
    {
        LOG_ERROR_FMT("Read failed with: \"{}\"", communication_status_to_str(status));
        co_return std::unexpected(status);
    }
}
CCharacteristic::awaitable_write_t CCharacteristic::write_data(const std::vector<uint8_t>& data) const
{
    co_return co_await write_data(data, GattWriteOption::WriteWithoutResponse);
}
CCharacteristic::awaitable_write_t CCharacteristic::write_data_with_response(const std::vector<uint8_t>& data) const
{
    co_return co_await write_data(data, GattWriteOption::WriteWithResponse);
}
CharacteristicProperties CCharacteristic::properties() const
{
    ASSERT(m_pCharacteristic, "Expected a valid characteristic.");
    return to_props_from_winrt(m_pCharacteristic->CharacteristicProperties());
}
std::vector<std::string> CCharacteristic::properties_as_str() const
{
    return properties_to_str(properties());
}
ProtectionLevel CCharacteristic::protection_level() const
{
    ASSERT(m_pCharacteristic, "Expected a valid characteristic.");
    return protection_level_from_winrt(m_pCharacteristic->ProtectionLevel());
}
void CCharacteristic::revoke_value_changed_handler()
{
    if (m_Revoker)
    {
        m_Revoker.revoke();
    }
}
void CCharacteristic::register_value_changed_handler()
{
    m_Revoker = m_pCharacteristic->ValueChanged(winrt::auto_revoke, value_changed_handler());
}
void CCharacteristic::refresh_value_changed_handler()
{
    revoke_value_changed_handler();
    register_value_changed_handler();
}
std::function<void(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic characteristic,
                   const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattValueChangedEventArgs& args)>
    CCharacteristic::value_changed_handler()
{
    return [this](GattCharacteristic characteristic, const GattValueChangedEventArgs& args)
    {
        winrt::Windows::Storage::Streams::IBuffer buffer = args.CharacteristicValue();
        this->m_NotifyEventHandler(std::span<uint8_t>{ buffer.data(), buffer.Length() });
    };
}
CCharacteristic::awaitable_write_t CCharacteristic::write_data(const std::vector<uint8_t>& data, GattWriteOption option) const
{
    using Buffer = winrt::Windows::Storage::Streams::Buffer;


    Buffer buffer{ static_cast<uint32_t>(data.size()) };
    buffer.Length(buffer.Capacity());
    std::memcpy(buffer.data(), data.data(), buffer.Length() <= data.size() ? buffer.Length() : data.size());

    try
    {
        co_return communication_status_from_winrt(co_await m_pCharacteristic->WriteValueAsync(buffer, option));
    }
    catch (...)    // catch all because i have no clue what windows throws and when
    {
        winrt::guid guid = m_pCharacteristic->Uuid();
        uint16_t uniqueValue = guid.Data1 & 0x00'00'FF'00;
        uniqueValue = uniqueValue | (guid.Data1 & 0x00'00'00'FF);

        LOG_ERROR_FMT("Windows BLE driver exception when trying to Write to characteristic with UUID: \"{}\"",
                      std::format("{:02X}", uniqueValue));

        co_return ble::CommunicationStatus::unreachable;
    }
}
winrt::Windows::Foundation::IAsyncAction CCharacteristic::query_descriptors()
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    using namespace winrt::Windows::Foundation::Collections;


    m_Descriptors.clear();

    GattDescriptorsResult result = co_await m_pCharacteristic->GetDescriptorsAsync();
    if (result.Status() == GattCommunicationStatus::Success)
    {
        IVectorView<GattDescriptor> descriptors = result.Descriptors();
        m_Descriptors.reserve(descriptors.Size());

        for (auto&& descriptor : descriptors)
        {
            auto [iter, emplaced] =
                m_Descriptors.try_emplace(make_uuid(descriptor.Uuid()),
                                          std::make_shared<CDescriptor>(co_await make_descriptor<CDescriptor>(descriptor)));
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
}    // namespace ble
