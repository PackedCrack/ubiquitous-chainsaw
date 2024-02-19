
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


namespace application 
{

//void nimble_initilize();
//void nimble_host_task(void* param);
//void nimble_start();

class CNimble 
{

public:
    CNimble();
    ~CNimble() = default;
    CNimble(const CNimble& other) = delete; // Copy constructor:
    CNimble(CNimble&& other) = delete; // Move constructor:
    CNimble& operator=(const CNimble& other) = delete; // copy assign
    CNimble& operator=(CNimble&& other) = delete; // move assign
public:
    static void host_task(void* param);
    void start();
private:
    void initilize();


};
} // namespace application