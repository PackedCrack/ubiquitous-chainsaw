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
CDescriptor::awaitable_make_t CDescriptor::make(const GattDescriptor& descriptor)
{
    // Work around because make_shared requires a public constructor
    // But construction of CDesriptor should go through this factory function
    co_return std::shared_ptr<CDescriptor>(new CDescriptor{ descriptor });
}
CDescriptor::CDescriptor(GattDescriptor descriptor)
    : m_Descriptor{ descriptor }
{
#ifndef NDEBUG
    LOG_INFO_FMT("Descriptor UUID: \"{}\"\nDescriptor protection level: \"{}\"",
                 winrt::to_string(to_hstring(m_Descriptor.Uuid())).c_str(),
                 prot_level_to_str(protection_level()));
#endif
}
CDescriptor::awaitable_read_t CDescriptor::read_value() const
{
    using GattReadResult = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattReadResult;
    using GattCommunicationStatus = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus;
    using BluetoothCacheMode = winrt::Windows::Devices::Bluetooth::BluetoothCacheMode;
    using IBuffer = winrt::Windows::Storage::Streams::IBuffer;


    try
    {
        GattReadResult result = co_await m_Descriptor.ReadValueAsync(BluetoothCacheMode::Uncached);

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
    catch (const winrt::hresult_error& err)
    {
        LOG_WARN_FMT("Exception: \"{:X}\" - \"{}\", thrown by WinRT when trying to read from Descriptor: \"{}\".",
                     err.code().value,
                     winrt::to_string(winrt::to_hstring(err.message())).c_str(),
                     winrt::to_string(winrt::to_hstring(m_Descriptor.Uuid())).c_str());
    }
    catch (...)
    {
        LOG_ERROR_FMT("Unknown Exception thrown by WinRT when trying to read from Descriptor: \"{}\"",
                      winrt::to_string(winrt::to_hstring(m_Descriptor.Uuid())).c_str());
    }

    co_return std::unexpected{ CommunicationStatus::unreachable };
}
std::string CDescriptor::uuid_as_str() const
{
    return winrt::to_string(winrt::to_hstring(m_Descriptor.Uuid()));
}
ProtectionLevel CDescriptor::protection_level() const
{
    return protection_level_from_winrt(m_Descriptor.ProtectionLevel());
}
}    // namespace ble
