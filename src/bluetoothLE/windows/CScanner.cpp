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
// TODO:: move this out cause all windows types might have to check it..
void init_com()
{
    static bool init = false;
    if(!init)
    {
        winrt::init_apartment();
        init = true;
    }
}

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
CScanner::CScanner(CThreadSafeHashMap<std::string, DeviceInfo>& deviceInfoCache)
    : m_Watcher{}
    , m_FoundDevices{ deviceInfoCache }
{
    init_com();
    
    m_Watcher.Received([this](auto&&, BluetoothLEAdvertisementReceivedEventArgs args)
    {
        DeviceInfo devInfo{};
        
        devInfo.addressType = address_type(args.BluetoothAddressType());
        if(devInfo.addressType == AddressType::none)
            devInfo.address = std::nullopt;
        else
            devInfo.address = std::make_optional(hex_addr_to_str(args.BluetoothAddress()));
        
        if(!m_FoundDevices.contains(devInfo.address.value()))
            m_FoundDevices.insert(hex_addr_to_str(args.BluetoothAddress()), std::move(devInfo));
    });
}
CScanner::~CScanner()
{
    if(m_Watcher.Status() == BluetoothLEAdvertisementWatcherStatus::Started)
        m_Watcher.Stop();
}

void CScanner::begin_scan() const
{
    m_Watcher.Start();
}
void CScanner::end_scan() const
{
    m_Watcher.Stop();
}
}   // namespace win
}   // namespace ble