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
    CScanner(CScanner&& other) = default;
    CScanner& operator=(const CScanner& other) = delete;
    CScanner& operator=(CScanner&& other) = default;
public:
    void begin_scan() const;
    void end_scan() const;
private:
    winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher m_Watcher;
    CThreadSafeHashMap<std::string, DeviceInfo>& m_FoundDevices;
};

}   // namespace win
}   // namespace ble