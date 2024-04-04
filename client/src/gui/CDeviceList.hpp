//
// Created by qwerty on 2024-04-03.
//

#pragma once
#include "../../common/Pointer.hpp"
#include "../bluetoothLE/Scanner.hpp"
#include "../bluetoothLE/Device.hpp"
#include "../common/CMutex.hpp"
#include "../common/CStopWatch.hpp"


namespace gui
{
class CDeviceList
{
    using time_t = std::chrono::seconds;
public:
    explicit CDeviceList(ble::CScanner& scanner);
    ~CDeviceList() = default;
    CDeviceList(const CDeviceList& other);
    CDeviceList(CDeviceList&& other) = default;
    CDeviceList& operator=(const CDeviceList& other);
    CDeviceList& operator=(CDeviceList&& other) = default;
private:
    void copy(const CDeviceList& other);
public:
    void push();
private:
    [[nodiscard]] auto time_limited_scan(std::chrono::seconds seconds);
    void new_scan();
    void device_list();
private:
    Pointer<ble::CScanner> m_pScanner = nullptr;
    std::vector<ble::DeviceInfo> m_Devices;
    std::unique_ptr<std::mutex> m_pMutex;
    common::CStopWatch<time_t> m_Timer;
private:
    using mutex_t = std::remove_cvref_t<decltype(*m_pMutex)>;
};
}   // namespace gui