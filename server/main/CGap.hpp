#pragma once

/* STD */
#include <string>
#include <cstdio>
#include <type_traits>
#include <vector>
#include <atomic>
#include <array>
#include <optional>


#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

/* Project */
#include "defines.hpp"
#include "ServerDefines.hpp"

/* BLE */
#include "host/ble_hs_adv.h"
#include "host/ble_gap.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"

//#include "nimble/nimble_port.h"


namespace ble
{


class CConnectionHandle // NOTE: Will this be needed for GATT services???? probably drop connectino if not authenticated
{


//struct BleCharacteristic
//{
//    ble_uuid_any_t uuid;
//    uint16_t handle;
//    uint16_t handleValue;
//    uint8_t properties;
//};
//
//struct BleService 
//{
//    ble_uuid_any_t uuid;
//    uint16_t connHandle;
//    uint16_t handleStart;
//    uint16_t handleEnd;
//    std::vector<BleCharacteristic> characteristics;
//};


public:
    CConnectionHandle();
    ~CConnectionHandle();
    CConnectionHandle(const CConnectionHandle& other) = delete;
    CConnectionHandle(CConnectionHandle&& other) = delete;
    CConnectionHandle& operator=(const CConnectionHandle& other) = delete;
    CConnectionHandle& operator=(CConnectionHandle&& other) = delete;

public:
    [[nodiscard]] uint16_t handle() const;
    [[nodiscard]] int drop(int reason);
    [[nodiscard]] int num_services() const;
    [[nodiscard]] const std::vector<BleService> services() const;
    void set_connection(uint16_t id);
    void reset_connection();
    void add_service(const BleService& service);


private:
    uint16_t m_id;
    std::vector<BleService> m_services;

};




class CGap
{
public:
    CGap();
    ~CGap() = default;
    CGap(const CGap& other) = delete;
    CGap(CGap&& other) = delete;
    CGap& operator=(const CGap& other) = delete;
    CGap& operator=(CGap&& other) = delete;
public:
    [[nodiscard]] uint16_t connection_handle() const ;
    [[nodiscard]] int drop_connection(int reason);
    [[nodiscard]] int discover_services();
    void set_connection(uint16_t id);
    void reset_connection();
    void start();
    void rssi();
    void begin_advertise();
    void end_advertise();
private:
    uint8_t m_bleAddressType;
    ble_gap_adv_params m_params;
    //std::atomic<bool> m_isAdvertising;
    CConnectionHandle m_currentConnectionHandle;
};

} // namespace nimble
