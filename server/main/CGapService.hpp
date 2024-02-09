#pragma once

/* STD */
#include <string>
#include "defines.hpp"
#include <cstdio>
#include <type_traits>
#include <functional>

/* Project */
#include "defines.hpp"

/* BLE */
#include "host/ble_hs_adv.h"
#include "host/ble_gap.h"
#include "services/gap/ble_svc_gap.h"
#include "host/ble_uuid.h"


namespace application
{

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
private:
    static int gap_event_handler(struct ble_gap_event *event, void *arg); // TODO have this in the gattserver and then pass it down
private:
    uint8_t m_bleAddressType;
    ble_gap_adv_params m_params;
    bool m_isAdvertising;
};
} // namespace nimble
