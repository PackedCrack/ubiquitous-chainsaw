//
// Created by qwerty on 2024-01-26.
//
#include "CScanner.hpp"
// windows
#include <winrt/Windows.Foundation.h>


using namespace winrt::Windows::Devices::Bluetooth;
using namespace winrt::Windows::Devices::Bluetooth::Advertisement;

namespace
{
[[nodiscard]] ble::AddressType address_type(BluetoothAddressType addressTypeWin)
{
    if(addressTypeWin == BluetoothAddressType::Public)
        return ble::AddressType::real;
    else if(addressTypeWin == BluetoothAddressType::Random)
        return ble::AddressType::random;
    else
        return ble::AddressType::none;
}


}   // namespace

namespace ble
{
namespace win
{
CScanner::CScanner()
    : m_Watcher{}
    , m_ReceivedRevoker{ m_Watcher.Received(winrt::auto_revoke, received_event_handler()) }
    , m_FoundDevices{}
{}
CScanner::~CScanner()
{
    if(m_Watcher == nullptr)
        return;
    
    if(m_Watcher.Status() == BluetoothLEAdvertisementWatcherStatus::Started)
        m_Watcher.Stop();
}

CScanner::CScanner(CScanner&& other) noexcept
        : m_Watcher{}
        , m_FoundDevices{ std::move(other.m_FoundDevices) }
{
    move_impl(other);
}

CScanner& CScanner::operator=(CScanner&& other) noexcept
{
    if(this != &other)
    {
        m_FoundDevices = std::move(other.m_FoundDevices);
        move_impl(other);
    }
    
    return *this;
}
void CScanner::move_impl(CScanner& other)
{
    other.m_Watcher.Stop(); // TODO:: Should probably wait for this?
    m_Watcher = std::move(other.m_Watcher);
    ASSERT(other.m_Watcher == nullptr, "Microsoft's type does not set itself to nullptr after move - do it manually");
    // We must update the event handlers since the this pointer has changed..
    refresh_received_event_handler();
}
void CScanner::begin_scan() const
{
    m_Watcher.Start();
}
void CScanner::end_scan() const
{
    m_Watcher.Stop();
}
std::vector<ble::DeviceInfo> CScanner::found_devices()
{
    return m_FoundDevices.as_vector();
}
void CScanner::revoke_received_event_handler()
{
    m_ReceivedRevoker.revoke();
}

void CScanner::register_received_event_handler()
{
    m_ReceivedRevoker = m_Watcher.Received(winrt::auto_revoke, received_event_handler());
}

void CScanner::refresh_received_event_handler()
{
    revoke_received_event_handler();
    register_received_event_handler();
}

std::function<void(const BluetoothLEAdvertisementWatcher&, BluetoothLEAdvertisementReceivedEventArgs)> CScanner::received_event_handler()
{
    return [this](auto&&, BluetoothLEAdvertisementReceivedEventArgs args)
    {
        DeviceInfo devInfo{};
        
        devInfo.addressType = address_type(args.BluetoothAddressType());
        if(devInfo.addressType == AddressType::none)
            devInfo.address = std::nullopt;
        else
            devInfo.address = std::make_optional(args.BluetoothAddress());
        
        // TODO:: We cant cache on address if the address is optional.
        std::string strAddress = ble::hex_addr_to_str(devInfo.address.value());
        if(!m_FoundDevices.contains(strAddress))
            m_FoundDevices.insert(std::move(strAddress), std::move(devInfo));
    };
}
}   // namespace win
}   // namespace ble