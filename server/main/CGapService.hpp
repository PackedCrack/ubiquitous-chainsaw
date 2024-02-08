#pragma once

/* STD */
#include <string>
#include "defines.hpp"
#include <cstdio>

/* Project */
#include "defines.hpp"

/* BLE */
#include "host/ble_hs_adv.h"
#include "host/ble_gap.h"
#include "services/gap/ble_svc_gap.h"
#include "host/ble_uuid.h"


namespace nimble
{

class CGapService 
{
public:
    CGapService() = default;
    CGapService(const std::string_view, const uint8_t addrType);
    ~CGapService() = default;
    CGapService(const CGapService& other) = default;
    CGapService(CGapService&& other) = default;
    CGapService& operator=(const CGapService& other) = default;
    CGapService& operator=(CGapService&& other) = default;

    [[NoDiscard]] uint8_t gap_param_is_alive();
private:
    static int gap_event_handler(struct ble_gap_event *event, void *arg); // TODO have this in the gattserver and then pass it down
private:
    uint8_t m_bleAddressType;
    ble_gap_adv_params m_params;


};
} // namespace nimble
