
#pragma once
/* STD */
#include <cstdio>
#include <array>
#include <future>

/* Project */
#include "defines.hpp"
#include "CGap.hpp"
#include "CGatt.hpp"

/* BLE */
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/util/util.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/ans/ble_svc_ans.h"

#include "host/ble_hs.h"

/* ESP */
#include "esp_log.h"


namespace ble 
{

class CNimble 
{

public:
    CNimble();
    ~CNimble();
    CNimble(const CNimble& other) = delete;
    CNimble(CNimble&& other) noexcept;
    CNimble& operator=(const CNimble& other) = delete;
    CNimble& operator=(CNimble&& other);
private:
    //CGatt m_gatt;

	//std::shared_ptr<CAuthentication> m_pAuthentication;
    CGap m_gap;


};
} // namespace application