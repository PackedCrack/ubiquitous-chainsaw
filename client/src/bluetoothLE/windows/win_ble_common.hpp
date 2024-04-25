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
    winrt_status_to_communication_status(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus status)
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;

    UNHANDLED_CASE_PROTECTION_ON
    switch (status)
    // cppcheck-suppress missingReturn
    {
    case GattCommunicationStatus::Unreachable:
        return ble::CommunicationStatus::unreachable;
    case GattCommunicationStatus::ProtocolError:
        return ble::CommunicationStatus::protocolError;
    case GattCommunicationStatus::AccessDenied:
        return ble::CommunicationStatus::accessDenied;
    case GattCommunicationStatus::Success:
        return ble::CommunicationStatus::success;
    }
    UNHANDLED_CASE_PROTECTION_OFF

    std::unreachable();
}
}    // namespace ble
