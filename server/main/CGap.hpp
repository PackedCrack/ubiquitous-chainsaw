#pragma once

/* STD */
#include <string>
#include <cstdio>
#include <type_traits>
#include <vector>
#include <atomic>
#include <array>

/* Project */
#include "defines.hpp"

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
    void start();
    void rssi();
    void begin_advertise();
    void end_advertise();
private:
    uint8_t m_bleAddressType;
    ble_gap_adv_params m_params;
    std::atomic<bool> m_isAdvertising;
};

} // namespace nimble
