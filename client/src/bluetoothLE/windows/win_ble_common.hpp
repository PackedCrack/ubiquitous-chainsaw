//
// Created by qwerty on 2024-04-21.
//

#pragma once
#include "../ble_common.hpp"
// third party
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
namespace ble
{
[[nodiscard]] constexpr ble::CommunicationStatus
    communication_status_from_winrt(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus status)
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;

    UNHANDLED_CASE_PROTECTION_ON
    switch (status)
    // cppcheck-suppress missingReturn
    {
        // clang-format off
    case GattCommunicationStatus::Unreachable: return ble::CommunicationStatus::unreachable;
    case GattCommunicationStatus::ProtocolError: return ble::CommunicationStatus::protocolError;
    case GattCommunicationStatus::AccessDenied: return ble::CommunicationStatus::accessDenied;
    case GattCommunicationStatus::Success: return ble::CommunicationStatus::success;
        // clang-format on
    }
    UNHANDLED_CASE_PROTECTION_OFF

    std::unreachable();
}
[[nodiscard]] constexpr ProtectionLevel
    protection_level_from_winrt(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattProtectionLevel level)
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;

    UNHANDLED_CASE_PROTECTION_ON
    switch (level)
    // cppcheck-suppress missingReturn
    {
        // clang-format off
    case GattProtectionLevel::AuthenticationRequired: return ProtectionLevel::authenticationRequired;
    case GattProtectionLevel::EncryptionAndAuthenticationRequired: return ProtectionLevel::encryptionAndAuthenticationRequired;
    case GattProtectionLevel::EncryptionRequired: return ProtectionLevel::encryptionRequired;
    case GattProtectionLevel::Plain: return ProtectionLevel::plain;
    // clang-format off
    }
    UNHANDLED_CASE_PROTECTION_OFF
    
    std::unreachable();
}
}    // namespace ble
