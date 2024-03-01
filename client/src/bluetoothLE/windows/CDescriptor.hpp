//
// Created by qwerty on 2024-03-01.
//
#pragma once
#include "defines.hpp"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>


namespace ble::win
{
enum class ProtectionLevel : int32_t
{
    plain = 0,
    authenticationRequired = 1,
    encryptionRequired = 2,
    encryptionAndAuthenticationRequired = 3,
};

class CDescriptor
{
public:
    explicit CDescriptor(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDescriptor descriptor);
    ~CDescriptor() = default;
    CDescriptor(const CDescriptor& other) = default;
    CDescriptor(CDescriptor&& other) = default;
    CDescriptor& operator=(const CDescriptor& other) = default;
    CDescriptor& operator=(CDescriptor&& other) = default;
public:
    [[nodiscard]] std::string uuid_as_str() const;
private:
    winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDescriptor m_Descriptor;
    ProtectionLevel m_ProtLevel;
};

[[nodiscard]] constexpr ProtectionLevel prot_level_from_winrt(
        winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattProtectionLevel level)
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    
    UNHANDLED_CASE_PROTECTION_ON
    switch (level)
    {
        case GattProtectionLevel::AuthenticationRequired:
            return ProtectionLevel::authenticationRequired;
        case GattProtectionLevel::EncryptionAndAuthenticationRequired:
            return ProtectionLevel::encryptionAndAuthenticationRequired;
        case GattProtectionLevel::EncryptionRequired:
            return ProtectionLevel::encryptionRequired;
        case GattProtectionLevel::Plain:
            return ProtectionLevel::plain;
    }
    UNHANDLED_CASE_PROTECTION_OFF
    
    std::unreachable();
}

[[nodiscard]] constexpr const char* prot_level_to_str(ProtectionLevel level)
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    
    UNHANDLED_CASE_PROTECTION_ON
    switch (level)
    {
        case ProtectionLevel::authenticationRequired:
            return "Authentication Required";
        case ProtectionLevel::encryptionAndAuthenticationRequired:
            return "Encryption and Authentication Required";
        case ProtectionLevel::encryptionRequired:
            return "Encryption Required";
        case ProtectionLevel::plain:
            return "Plain";
    }
    UNHANDLED_CASE_PROTECTION_OFF
    
    std::unreachable();
}
}   // namespace ble::win