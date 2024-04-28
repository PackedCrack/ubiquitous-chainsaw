//
// Created by qwerty on 2024-03-01.
//
#include "CDescriptor.hpp"
#include "win_ble_common.hpp"
// winrt
#include <winrt/Windows.Storage.Streams.h>
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
CDescriptor::awaitable_read_t CDescriptor::read_value() const
{
    using GattReadResult = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattReadResult;
    using GattCommunicationStatus = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus;
    using BluetoothCacheMode = winrt::Windows::Devices::Bluetooth::BluetoothCacheMode;
    using IBuffer = winrt::Windows::Storage::Streams::IBuffer;


    GattReadResult result = co_await m_pDescriptor->ReadValueAsync(BluetoothCacheMode::Uncached);
    if (result.Status() == GattCommunicationStatus::Success)
    {
        std::expected<std::vector<uint8_t>, CommunicationStatus> expected{};
        IBuffer buffer = result.Value();
        expected->resize(static_cast<std::size_t>(buffer.Length()));
        ASSERT(expected->size() == buffer.Length(), "Expected equal size..");
        std::memcpy(expected->data(), buffer.data(), buffer.Length());

        co_return expected;
    }
    else
    {
        co_return std::unexpected{ communication_status_from_winrt(result.Status()) };
    }
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
