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
public:
    using pos_t = std::vector<ble::DeviceInfo>::difference_type;
private:
    using IBluetoothLEAdvertisementWatcher = winrt::Windows::Devices::Bluetooth::Advertisement::IBluetoothLEAdvertisementWatcher;
    using BluetoothLEAdvertisementWatcher = winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher;
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
    void begin_scan();
    void end_scan() const;
    [[nodiscard]] std::vector<ble::DeviceInfo> retrieve_n_devices(pos_t index, pos_t n) const;
    [[nodiscard]] const std::atomic<size_t>& num_devices() const;
    [[nodiscard]] bool scanning() const;
private:
    void move_impl(CScanner& other);
    void revoke_received_event_handler();
    void register_received_event_handler();
    void refresh_received_event_handler();
    [[nodiscard]] std::function<void(const BluetoothLEAdvertisementWatcher&, BluetoothLEAdvertisementReceivedEventArgs)>
        received_event_handler();
private:
    BluetoothLEAdvertisementWatcher m_Watcher;
    winrt::event_revoker<IBluetoothLEAdvertisementWatcher> m_ReceivedRevoker;
    std::vector<DeviceInfo> m_FoundDevices;
    std::unordered_set<int64_t> m_DeviceCache;
    std::atomic<size_t> m_Count;
    std::unique_ptr<std::mutex> m_pMutex;
};
}    // namespace ble
