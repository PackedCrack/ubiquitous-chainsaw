//
// Created by qwerty on 2024-03-01.
//

#pragma
#include "defines.hpp"
#include "../../common/ble_services.hpp"
#include "CDescriptor.hpp"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>


namespace ble
{
class CCharacteristic
{
public:
    enum class State : uint32_t
    {
        uninitialized,
        queryingDescriptors,
        ready
    };
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
        reliableWrites = 0x100,
        writableAuxiliaries = 0x200,
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
    [[nodiscard]] friend constexpr Properties operator~(Properties prop)
    {
        return Properties{ std::to_underlying(prop) };
    }
public:
    [[nodiscard]] static CCharacteristic make_characteristic(
            const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic& characteristic);
    ~CCharacteristic() = default;
    CCharacteristic(const CCharacteristic& other) = default;
    CCharacteristic(CCharacteristic&& other) = default;
    CCharacteristic& operator=(const CCharacteristic& other) = default;
    CCharacteristic& operator=(CCharacteristic&& other) = default;
public:
    [[nodiscard]] std::string uuid_as_str() const;
    [[nodiscard]] bool ready() const;
    [[nodiscard]] State state() const;
    winrt::Windows::Foundation::IAsyncAction query_descriptors();
    void read_value() const;
private:
    explicit CCharacteristic(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic characteristic);
    winrt::Windows::Foundation::IAsyncAction init();
public:
    winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic m_Characteristic;
    std::unordered_map<ble::UUID, CDescriptor, ble::UUID::Hasher> m_Descriptors;
    ProtectionLevel m_ProtLevel;
    Properties m_Properties;
    State m_State;
};

constexpr const char* gatt_communication_status_to_str(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus status)
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    
    UNHANDLED_CASE_PROTECTION_ON
    switch(status)
    // cppcheck-suppress missingReturn
    {
        case GattCommunicationStatus::Unreachable:
            return "Unreachable";
        case GattCommunicationStatus::ProtocolError:
            return "Protocol Error";
        case GattCommunicationStatus::AccessDenied:
            return "Access Denied";
        case GattCommunicationStatus::Success:
            return "Success";
    }
    UNHANDLED_CASE_PROTECTION_OFF
    
    std::unreachable();
}
}   // namespace ble
