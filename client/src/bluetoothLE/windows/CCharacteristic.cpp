//
// Created by qwerty on 2024-03-01.
//

#include "CCharacteristic.hpp"
#include "../common.hpp"
#include "../../common/common.hpp"


#define TO_BOOL(expr) common::enum_to_bool(expr)

namespace
{
[[nodiscard]] ble::win::CCharacteristic::Properties operator|(
        ble::win::CCharacteristic::Properties lhs,
        winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties rhs)
{
    return ble::win::CCharacteristic::Properties{ std::to_underlying(lhs) | std::to_underlying(rhs) };
}
[[nodiscard]] std::vector<std::string> properties_to_str(ble::win::CCharacteristic::Properties properties)
{
    using Properties = ble::win::CCharacteristic::Properties;
    
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
[[nodiscard]] ble::win::CCharacteristic::Properties to_props_from_winrt(
        winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties properties)
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    using Properties = ble::win::CCharacteristic::Properties;
    
    Properties props{ 0 };
    if(TO_BOOL(properties & GattCharacteristicProperties::AuthenticatedSignedWrites))
        props = props | GattCharacteristicProperties::AuthenticatedSignedWrites;
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
namespace ble::win
{
CCharacteristic CCharacteristic::make_characteristic(
        const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic& characteristic)
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    
    CCharacteristic charac{ characteristic };
    charac.init();
    
    return charac;
}
CCharacteristic::CCharacteristic(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic characteristic)
        : m_Characteristic{ std::move(characteristic) }
        , m_ProtLevel{}
        , m_Properties{}
{}
winrt::Windows::Foundation::IAsyncAction CCharacteristic::init()
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    
    std::printf("\nCharacteristic UUID: %ws", to_hstring(m_Characteristic.Uuid()).data());
    
    // Storing this mostly for debug purposes for now..
    m_Properties = to_props_from_winrt(m_Characteristic.CharacteristicProperties());
    std::vector<std::string> properties = properties_to_str(m_Properties);
    std::printf("\nCharacteristic properties: ");
    for(auto&& property : properties)
    {
        std::printf("%s, ", property.c_str());
    }
    // TODO:: might not need this
    m_ProtLevel = prot_level_from_winrt(m_Characteristic.ProtectionLevel());
    // TODO:: Debug print
    std::printf("\n%s", std::format("Characteristic protection level: \"{}\"", prot_level_to_str(m_ProtLevel)).c_str());
    
    
    GattDescriptorsResult result = co_await m_Characteristic.GetDescriptorsAsync();
    if(result.Status() == GattCommunicationStatus::Success)
    {
        winrt::Windows::Foundation::Collections::IVectorView<GattDescriptor> descriptors = result.Descriptors();
        for(auto&& descriptor : descriptors)
        {
            auto[iter, emplaced] = m_Descriptors.try_emplace(make_uuid(descriptor.Uuid()), CDescriptor{ descriptor });
            if(!emplaced)
            {
                LOG_ERROR("Failed to empalce descriptor");
            }
        }
    }
    else
    {
        LOG_WARN_FMT("failed to get descriptors. Failed with: \"{}\"", std::to_underlying(result.Status()));
    }
}

}   // namespace ble::win