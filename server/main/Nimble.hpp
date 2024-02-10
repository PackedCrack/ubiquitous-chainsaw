#pragma once
/* STD */
#include <cstdio>
#include <array>
#include <future>
#include <functional>

/* Project */
#include "defines.hpp"

/* BLE */
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/util/util.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/ans/ble_svc_ans.h"

#include "host/ble_hs.h"





/* ESP */
#include "esp_log.h"


namespace nimble 
{

void configure_nimble_host();
void wait_for_sync();
void nimble_host_task(void* param);

//class CNimble 
//{
//
//public:
//    CNimble();
//    ~CNimble();
//    CNimble(const CNimble& other) = delete; // Copy constructor:
//    CNimble(CNimble&& other) = delete; // Move constructor:
//    CNimble& operator=(const CNimble& other) = delete; // copy assign
//    CNimble& operator=(CNimble&& other) = delete; // move assign
//public:
//    static void task(void* param);
//	void wait_for_sync();
//private:
//    void configure_nimble_host();
//
//
//};
} // namespace nimble

