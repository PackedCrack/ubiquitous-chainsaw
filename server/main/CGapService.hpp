#pragma once

/* STD */
#include <string>
#include "defines.hpp"
#include <cstdio>
#include <type_traits>
#include <functional>
#include <vector>
#include <atomic>

/* Project */
#include "defines.hpp"

/* BLE */
#include "host/ble_hs_adv.h"
#include "host/ble_gap.h"
#include "services/gap/ble_svc_gap.h"
#include "host/ble_uuid.h"

#include "services/gatt/ble_svc_gatt.h"

//#include "nimble/nimble_port.h"


namespace application
{


//class CAdvertiser
//{
//public:
//    CAdvertiser();
//    ~CAdvertiser();
//    CAdvertiser(const CAdvertiser& other) = delete;
//    CAdvertiser(CAdvertiser&& other) = delete;
//    CAdvertiser& operator=(const CAdvertiser& other) = delete;
//
//public:
//    void begin_advertise();
//    void end_advertise();
//
//private:
//    ble_gap_adv_params m_params;
//};




// Probably not needed to be a class?
// no "resources" to keep track of really, except     uint8_t m_bleAddressType and ble_gap_adv_params m_params;
// nothing RAAI related that needs to be destroyed etc..

class CGapService 
{
public:
    CGapService();
    ~CGapService() = default;
    CGapService(const CGapService& other) = delete;
    CGapService(CGapService&& other) = delete;
    CGapService& operator=(const CGapService& other) = delete;
    CGapService& operator=(CGapService&& other) = delete;
public:
    void initilize(const std::string_view deviceName, uint8_t addressType);
    void rssi();

    void begin_advertise();
    void end_advertise();
private:
    uint8_t m_bleAddressType;
    ble_gap_adv_params m_params;
    std::atomic<bool> m_isAdvertising;
};
} // namespace nimble
