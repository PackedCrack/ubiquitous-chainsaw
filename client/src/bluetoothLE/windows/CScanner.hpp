//
// Created by qwerty on 2024-01-26.
//
#pragma once
#include "../ble_common.hpp"
#include "../../common/CThreadSafeHashMap.hpp"
// windows
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>


namespace ble
{
class CScanner
{
private:
    using IBluetoothLEAdvertisementWatcher = winrt::Windows::Devices::Bluetooth::Advertisement::IBluetoothLEAdvertisementWatcher;
    using BluetoothLEAdvertisementWatcher =
            winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher;
    using BluetoothLEAdvertisementReceivedEventArgs =
            winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs;
public:
    CScanner();
    ~CScanner();
    CScanner(const CScanner& other) = delete;
    CScanner(CScanner&& other) noexcept;
    CScanner& operator=(const CScanner& other) = delete;
    CScanner& operator=(CScanner&& other) noexcept;
public:
    void begin_scan() const;
    void end_scan() const;
    [[nodiscard]] std::vector<ble::DeviceInfo> found_devices();
private:
    void move_impl(CScanner& other);
    void revoke_received_event_handler();
    void register_received_event_handler();
    void refresh_received_event_handler();
    [[nodiscard]] std::function<void(
            const BluetoothLEAdvertisementWatcher&,
            BluetoothLEAdvertisementReceivedEventArgs)> received_event_handler();
private:
    BluetoothLEAdvertisementWatcher m_Watcher;
    winrt::event_revoker<IBluetoothLEAdvertisementWatcher> m_ReceivedRevoker;
    CThreadSafeHashMap<std::string, DeviceInfo> m_FoundDevices;
};
}   // namespace ble