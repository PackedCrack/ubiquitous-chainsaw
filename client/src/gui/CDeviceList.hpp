//
// Created by qwerty on 2024-04-03.
//

#pragma once
#include "common/Pointer.hpp"
#include "../bluetoothLE/Scanner.hpp"
#include "../bluetoothLE/Device.hpp"
#include "../common/CMutex.hpp"
#include "../common/CStopWatch.hpp"


namespace gui
{
class CDeviceList
{
    using time_t = std::chrono::seconds;
    using mutex_t = std::mutex;
public:
    static constexpr std::string_view KEY = "devicelist";
public:
    explicit CDeviceList(ble::CScanner& scanner);
    ~CDeviceList() = default;
    CDeviceList(const CDeviceList& other);
    CDeviceList(CDeviceList&& other) noexcept;
    CDeviceList& operator=(const CDeviceList& other);
    CDeviceList& operator=(CDeviceList&& other) noexcept;
private:
    void copy(const CDeviceList& other);
public:
    void push();
    [[nodiscard]] std::vector<ble::DeviceInfo> device_infos() const;
private:
    [[nodiscard]] auto time_limited_scan(std::chrono::seconds seconds);
    void new_scan();
    void device_list();
private:
    Pointer<ble::CScanner> m_pScanner = nullptr;
    std::vector<ble::DeviceInfo> m_Devices;
    std::unique_ptr<mutex_t> m_pMutex;
    common::CStopWatch<time_t> m_Timer;
};
}   // namespace gui