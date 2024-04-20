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
CScanner::CScanner()
    : m_Watcher{}
    , m_ReceivedRevoker{ m_Watcher.Received(winrt::auto_revoke, received_event_handler()) }
    , m_FoundDevices{}
    , m_DeviceCache{}
    , m_Count{ 0 }
    , m_pMutex{ std::make_unique<std::remove_cvref_t<decltype(*m_pMutex)>>() }
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
        , m_ReceivedRevoker{ std::move(other.m_ReceivedRevoker) }
        , m_FoundDevices{ std::move(other.m_FoundDevices) }
        , m_DeviceCache{ std::move(other.m_DeviceCache) }
        , m_Count{ m_FoundDevices.size() }
        , m_pMutex{ std::move(other.m_pMutex) }
{
    move_impl(other);
}
CScanner& CScanner::operator=(CScanner&& other) noexcept
{
    if(this != &other)
    {
        m_FoundDevices = std::move(other.m_FoundDevices);
        m_ReceivedRevoker = std::move(other.m_ReceivedRevoker);
        m_pMutex = std::move(other.m_pMutex);
        m_Count = m_FoundDevices.size();
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
void CScanner::begin_scan()
{
    m_Count.store(0u);
    m_FoundDevices.clear();
    m_DeviceCache.clear();
    m_Watcher.Start();
}
void CScanner::end_scan() const
{
    m_Watcher.Stop();
}
std::vector<ble::DeviceInfo> CScanner::retrieve_n_devices(pos_t index, pos_t n) const
{
    std::lock_guard<decltype(*m_pMutex)> lock{ *m_pMutex };
    ASSERT_FMT(index < static_cast<int64_t>(m_FoundDevices.size()),
               "Cannot retrieve {} + {} device infos since it is out of bounds!", index, n);
    
    auto begin = std::begin(m_FoundDevices) + index;
    pos_t end = std::min(n, static_cast<pos_t>(m_FoundDevices.size()));
    
    return std::vector<ble::DeviceInfo>{ begin, begin + end };
}
const std::atomic<size_t>& CScanner::num_devices() const
{
    return m_Count;
}
bool CScanner::scanning() const
{
    return m_Watcher.Status() == BluetoothLEAdvertisementWatcherStatus::Started;
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
    return [this](auto&&, const BluetoothLEAdvertisementReceivedEventArgs& args)
    {
        std::lock_guard<decltype(*m_pMutex)> lock{ *m_pMutex };
        int64_t address = args.BluetoothAddress();
        if(m_DeviceCache.contains(address))
            return;
        
        m_DeviceCache.emplace(address);
        m_FoundDevices.emplace_back();
        DeviceInfo& devInfo = m_FoundDevices.back();
        
        devInfo.addressType = address_type(args.BluetoothAddressType());
        if(devInfo.addressType == AddressType::none)
            devInfo.address = std::nullopt;
        else
            devInfo.address = std::make_optional(address);
        
        
        
        m_Count.fetch_add(1u);
        m_Count.notify_all();
    };
}
}   // namespace ble