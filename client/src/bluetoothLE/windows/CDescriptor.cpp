//
// Created by qwerty on 2024-03-01.
//
#include "CDescriptor.hpp"
#include "win_ble_common.hpp"
// clang-format off


// clang-format on
namespace ble
{
CDescriptor::awaitable_t CDescriptor::make(const GattDescriptor& descriptor)
{
    co_return CDescriptor{ descriptor };
}
CDescriptor::CDescriptor(GattDescriptor descriptor)
    : m_pDescriptor{ std::make_shared<GattDescriptor>(std::move(descriptor)) }
{
#ifndef NDEBUG
    std::printf("\nDescriptor UUID: %ws", to_hstring(m_pDescriptor->Uuid()).data());
    std::printf("\n%s", std::format("Descriptor protection level: \"{}\"", prot_level_to_str(protection_level())).data());
#endif
}
std::string CDescriptor::uuid_as_str() const
{
    return winrt::to_string(winrt::to_hstring(m_pDescriptor->Uuid()));
}
ProtectionLevel CDescriptor::protection_level() const
{
    return protection_level_from_winrt(m_pDescriptor->ProtectionLevel());
}
}    // namespace ble
