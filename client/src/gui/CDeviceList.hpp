//
// Created by qwerty on 2024-04-03.
//
#pragma once
#include "../CServer.hpp"
#include "taskflow/taskflow.hpp"
#include "common/Pointer.hpp"
#include "security/ecc_key.hpp"
#include "../bluetoothLE/Scanner.hpp"
#include "../bluetoothLE/Device.hpp"
#include "../common/CMutex.hpp"
#include "../common/CStopWatch.hpp"
#include "../system/System.hpp"
//
//
//
//
namespace gui
{
class CDeviceList
{
    using time_t = std::chrono::seconds;
    using mutex_t = std::mutex;
public:
    static constexpr std::string_view KEY = "devicelist";
    static constexpr std::chrono::seconds SCAN_TIME{ 25 };
public:
    explicit CDeviceList(ble::CScanner& scanner, CServer& server);
    ~CDeviceList();
    CDeviceList(const CDeviceList& other);
    CDeviceList(CDeviceList&& other) noexcept;
    CDeviceList& operator=(const CDeviceList& other);
    CDeviceList& operator=(CDeviceList&& other) noexcept;
private:
    void copy(const CDeviceList& other);
public:
    void push();
    void recreate_list();
    void clear_list();
    [[nodiscard]] std::vector<ble::DeviceInfo> device_infos() const;
private:
    [[nodiscard]] auto time_limited_scan(std::chrono::seconds seconds);
    void spawn_time_limited_scan();
    void end_time_limited_scan();
    void authentication_status();
    void device_list();
private:
    Pointer<ble::CScanner> m_pScanner = nullptr;
    Pointer<CServer> m_pServer = nullptr;
    std::vector<ble::DeviceInfo> m_Devices;
    std::unique_ptr<mutex_t> m_pMutex;
    common::CStopWatch<time_t> m_ScanTimer;
};
}    // namespace gui
