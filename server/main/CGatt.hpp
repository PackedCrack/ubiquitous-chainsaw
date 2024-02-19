#pragma once
#include <array>

#include "defines.hpp"

#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/ans/ble_svc_ans.h"


namespace application
{

class CGatt // TODO refactor maybe/probably, have to see after introducing auth protocol etc
{
public:
    CGatt();
    ~CGatt() = default;
    CGatt(const CGatt& other) = delete; // Copy constructor:
    CGatt(CGatt&& other) = delete; // Move constructor:
    CGatt& operator=(const CGatt& other) = delete; // copy assign
    CGatt& operator=(CGatt&& other) = delete; // move assign
};

}