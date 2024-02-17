//
// Created by qwerty on 2024-01-26.
//
#pragma once
#include "../common.hpp"
#include "../../common/CThreadSafeHashMap.hpp"
// windows
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>


namespace ble
{
namespace win
{
class CScanner
{
public:
    explicit CScanner(CThreadSafeHashMap<std::string, DeviceInfo>& deviceInfoCache);
    ~CScanner();
    CScanner(const CScanner& other) = delete;
    CScanner(CScanner&& other) noexcept;
    CScanner& operator=(const CScanner& other) = delete;
    CScanner& operator=(CScanner&& other) noexcept;
public:
    void begin_scan() const;
    void end_scan() const;
private:
    void revoke_received_event_handler();
    void register_received_event_handler();
    void refresh_received_event_handler();
    [[nodiscard]] std::function<void(
            const winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher&,
            winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs)> received_event_handler();
private:
    winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher m_Watcher;
    winrt::event_revoker<winrt::Windows::Devices::Bluetooth::Advertisement::IBluetoothLEAdvertisementWatcher> m_ReceivedRevoker;
    CThreadSafeHashMap<std::string, DeviceInfo>* m_pFoundDevices;
};
}   // namespace win
}   // namespace ble