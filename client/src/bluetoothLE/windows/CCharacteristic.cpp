//
// Created by qwerty on 2024-03-01.
//

#include "CCharacteristic.hpp"
#include "win_ble_common.hpp"

#include <winrt/Windows.Storage.Streams.h>
#include <iostream>

#define TO_BOOL(expr) common::enum_to_bool(expr)

namespace
{
[[nodiscard]] ble::CCharacteristic::Properties operator|(
        ble::CCharacteristic::Properties lhs,
        winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties rhs)
{
    return ble::CCharacteristic::Properties{ std::to_underlying(lhs) | std::to_underlying(rhs) };
}
[[nodiscard]] std::vector<std::string> properties_to_str(ble::CCharacteristic::Properties properties)
{
    using Properties = ble::CCharacteristic::Properties;
    
    std::vector<std::string>  props{};
    if(static_cast<bool>(properties & Properties::authenticatedSignedWrites))
        props.emplace_back("Authenticated Signed Writes");
    if(TO_BOOL(properties & Properties::broadcast))
        props.emplace_back("Broadcast");
    if(TO_BOOL(properties & Properties::extendedProperties))
        props.emplace_back("Extended Properties");
    if(TO_BOOL(properties & Properties::indicate))
        props.emplace_back("Indicate");
    if(TO_BOOL(properties & Properties::none))
        props.emplace_back("None");
    if(TO_BOOL(properties & Properties::notify))
        props.emplace_back("Notify");
    if(TO_BOOL(properties & Properties::read))
        props.emplace_back("Read");
    if(TO_BOOL(properties & Properties::reliableWrites))
        props.emplace_back("Reliable Writes");
    if(TO_BOOL(properties & Properties::writableAuxiliaries))
        props.emplace_back("Writable Auxiliaries");
    if(TO_BOOL(properties & Properties::write))
        props.emplace_back("Write");
    if(TO_BOOL(properties & Properties::writeWithoutResponse))
        props.emplace_back("Write Without Response");
    
    
    return props;
}
[[nodiscard]] ble::CCharacteristic::Properties to_props_from_winrt(
        winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties properties)
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    using Properties = ble::CCharacteristic::Properties;
    
    Properties props{ 0 };
    if(TO_BOOL(properties & GattCharacteristicProperties::AuthenticatedSignedWrites))
    {
        // cppcheck-suppress badBitmaskCheck
        props = props | GattCharacteristicProperties::AuthenticatedSignedWrites;
    }
    if(TO_BOOL(properties & GattCharacteristicProperties::Broadcast))
        props = props | GattCharacteristicProperties::Broadcast;
    if(TO_BOOL(properties & GattCharacteristicProperties::ExtendedProperties))
        props = props | GattCharacteristicProperties::ExtendedProperties;
    if(TO_BOOL(properties & GattCharacteristicProperties::Indicate))
        props = props | GattCharacteristicProperties::Indicate;
    if(TO_BOOL(properties & GattCharacteristicProperties::None))
        props = props | GattCharacteristicProperties::None;
    if(TO_BOOL(properties & GattCharacteristicProperties::Notify))
        props = props | GattCharacteristicProperties::Notify;
    if(TO_BOOL(properties & GattCharacteristicProperties::Read))
        props = props | GattCharacteristicProperties::Read;
    if(TO_BOOL(properties & GattCharacteristicProperties::ReliableWrites))
        props = props | GattCharacteristicProperties::ReliableWrites;
    if(TO_BOOL(properties & GattCharacteristicProperties::WritableAuxiliaries))
        props = props | GattCharacteristicProperties::WritableAuxiliaries;
    if(TO_BOOL(properties & GattCharacteristicProperties::Write))
        props = props | GattCharacteristicProperties::Write;
    if(TO_BOOL(properties & GattCharacteristicProperties::WriteWithoutResponse))
        props = props | GattCharacteristicProperties::WriteWithoutResponse;
    
    return props;
}
}   // namespace
namespace ble
{
CCharacteristic::awaitable_t CCharacteristic::make(const GattCharacteristic& characteristic)
{
    CCharacteristic charac{ characteristic };
    
    // Storing this mostly for debug purposes for now..
    charac.m_Properties = to_props_from_winrt(charac.m_pCharacteristic->CharacteristicProperties());
    std::vector<std::string> properties = properties_to_str(charac.m_Properties);
    // TODO:: might not need this
    charac.m_ProtLevel = prot_level_from_winrt(charac.m_pCharacteristic->ProtectionLevel());
    
    #ifndef NDEBUG
    std::printf("\nCharacteristic UUID: %ws", to_hstring(charac.m_pCharacteristic->Uuid()).data());

    std::printf("\nCharacteristic properties: ");
    for (auto&& property : properties)
    {
        std::printf("%s, ", property.c_str());
    }

    std::printf("\n%s", std::format("Characteristic protection level: \"{}\"", prot_level_to_str(charac.m_ProtLevel)).c_str());
    #endif

    co_await charac.query_descriptors();
    co_return charac;
}
CCharacteristic::CCharacteristic(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic characteristic)
        : m_pCharacteristic{ std::make_shared<GattCharacteristic>(std::move(characteristic)) }
        , m_ProtLevel{}
        , m_Properties{}
{}
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
    
    CommunicationStatus status = winrt_status_to_communication_status(result.Status());
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
        LOG_ERROR_FMT("Read failed with: \"{}\"", gatt_communication_status_to_str(status));
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
CCharacteristic::awaitable_write_t CCharacteristic::write_data(const std::vector<uint8_t>& data, GattWriteOption option) const
{
    using GattCommunicationStatus = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus;
    using Buffer = winrt::Windows::Storage::Streams::Buffer;
    
    
    Buffer buffer{ static_cast<uint32_t>(data.size()) };
    buffer.Length(buffer.Capacity());
    std::memcpy(buffer.data(), data.data(), buffer.Length() <= data.size() ? buffer.Length() : data.size());
    
    
    try
    {
        co_return winrt_status_to_communication_status(co_await m_pCharacteristic->WriteValueAsync(buffer, option));
    }
    catch (...)    // catch all because i have no clue what windows throws and when
    {
        winrt::guid guid = m_pCharacteristic->Uuid();
        uint16_t uniqueValue = guid.Data1 & 0x0000'FF00;
        uniqueValue = uniqueValue | (guid.Data1 & 0x0000'00FF);

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
    if(result.Status() == GattCommunicationStatus::Success)
    {
        IVectorView<GattDescriptor> descriptors = result.Descriptors();
        m_Descriptors.reserve(descriptors.Size());
        
        for(auto&& descriptor : descriptors)
        {
            auto[iter, emplaced] =
                    m_Descriptors.try_emplace(make_uuid(descriptor.Uuid()), co_await make_descriptor<CDescriptor>(descriptor));
            if(!emplaced)
            {
                LOG_ERROR_FMT("Failed to emplace descriptor with UUID: \"{}\"", uuid_as_str());
            }
        }
    }
    else
    {
        LOG_ERROR_FMT("Communication error: \"{}\" when trying to query Descriptors from Characteristic with UUID: \"{}\"",
                      gatt_communication_status_to_str(winrt_status_to_communication_status(result.Status())),
                      uuid_as_str());
    }
}
}   // namespace ble