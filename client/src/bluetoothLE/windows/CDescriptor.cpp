//
// Created by qwerty on 2024-03-01.
//

#include "CDescriptor.hpp"


namespace ble
{
CDescriptor::awaitable_t CDescriptor::make(const GattDescriptor& descriptor)
{
    co_return CDescriptor{ descriptor };
}
CDescriptor::CDescriptor(GattDescriptor descriptor)
    : m_pDescriptor{ std::make_shared<GattDescriptor>(std::move(descriptor)) }
    , m_ProtLevel{ prot_level_from_winrt(m_pDescriptor->ProtectionLevel()) }
{
    // TODO:: Debug print
    std::printf("\nDescriptor UUID: %ws", to_hstring(m_pDescriptor->Uuid()).data());
    std::printf("\n%s", std::format("Descriptor protection level: \"{}\"", prot_level_to_str(m_ProtLevel)).c_str());
}
std::string CDescriptor::uuid_as_str() const
{
    return winrt::to_string(winrt::to_hstring(m_pDescriptor->Uuid()));
}
ProtectionLevel CDescriptor::protection_level() const
{
    return m_ProtLevel;
}
}   // namespace ble