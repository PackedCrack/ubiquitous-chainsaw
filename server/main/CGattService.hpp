#pragma once
#include "defines.hpp"


#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/ans/ble_svc_ans.h"


namespace application
{

class CGattService
{
public:
    CGattService();
    ~CGattService() = default;
    CGattService(const CGattService& other) = delete; // Copy constructor:
    CGattService(CGattService&& other) = delete; // Move constructor:
    CGattService& operator=(const CGattService& other) = delete; // copy assign
    CGattService& operator=(CGattService&& other) = delete; // move assign
};

}