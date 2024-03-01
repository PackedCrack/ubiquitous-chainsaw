//
// Created by qwerty on 2024-03-01.
//

#include "CDescriptor.hpp"


namespace ble::win
{
CDescriptor::CDescriptor(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDescriptor descriptor)
    : m_Descriptor{ std::move(descriptor) }
    , m_ProtLevel{ prot_level_from_winrt(m_Descriptor.ProtectionLevel()) }
{
    // TODO:: Debug print
    std::printf("\nDescriptor UUID: %ws", to_hstring(m_Descriptor.Uuid()).data());
    std::printf("\n%s", std::format("Descriptor protection level: \"{}\"", prot_level_to_str(m_ProtLevel)).c_str());
}
}   // namespace ble::win