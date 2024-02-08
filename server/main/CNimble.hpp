#pragma once
/* STD */
#include <cstdio>
#include <array>

/* Project */
#include "defines.hpp"
#include "CGattServer.hpp"

/* BLE */
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/util/util.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/ans/ble_svc_ans.h"

/* ESP */
#include "esp_log.h"

namespace nimble 
{
class CNimble 
{
public:
    CNimble();
    ~CNimble();
    CNimble(const CNimble& other) = delete; // Copy constructor:
    CNimble(CNimble&& other) = delete; // Move constructor:
    CNimble& operator=(const CNimble& other) = delete; // copy assign
    CNimble& operator=(CNimble&& other) = delete; // move assign

    void start();
    static void task(void* param);
    bool isInitilized();

    ///
    [[NoDiscard]] uint8_t gap_param_is_alive();

private:
    void configure_nimble_host();
    static void ble_on_reset_event_handle(int reason);
    static void ble_gatt_service_register_event_handle(struct ble_gatt_register_ctxt *ctxt, void *arg);
private:
    CGattServer m_gattServer;
};
} // namespace nimble

