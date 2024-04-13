//
// Created by qwerty on 2024-03-01.
//
#pragma once
#include "../../client_defines.hpp"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <pplawait.h>


namespace ble
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
    using awaitable_t = concurrency::task<CDescriptor>;
private:
    using GattDescriptor = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDescriptor;
public:
    [[nodiscard]] static awaitable_t make(const GattDescriptor& descriptor);
    CDescriptor() = default;
    ~CDescriptor() = default;
    CDescriptor(const CDescriptor& other) = default;
    CDescriptor(CDescriptor&& other) = default;
    CDescriptor& operator=(const CDescriptor& other) = default;
    CDescriptor& operator=(CDescriptor&& other) = default;
private:
    explicit CDescriptor(GattDescriptor descriptor);
public:
    [[nodiscard]] std::string uuid_as_str() const;
    [[nodiscard]] ProtectionLevel protection_level() const;
private:
    std::shared_ptr<GattDescriptor> m_pDescriptor;
    ProtectionLevel m_ProtLevel = ProtectionLevel::plain;
};

[[nodiscard]] constexpr ProtectionLevel prot_level_from_winrt(
        winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattProtectionLevel level)
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    
    UNHANDLED_CASE_PROTECTION_ON
    switch (level)
    // cppcheck-suppress missingReturn
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
    // cppcheck-suppress missingReturn
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
}   // namespace ble