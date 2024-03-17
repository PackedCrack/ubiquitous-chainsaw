#pragma once
#include <array>
#include <vector>

// project
#include "ble_common.hpp"
#include "defines.hpp"

#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/ans/ble_svc_ans.h"


namespace ble
{

struct BleService
{
    std::array<ble_gatt_dsc_def, 2> desc;
    std::array<ble_gatt_chr_def, 2> chr;
    std::array<ble_gatt_svc_def, 2> svc;
};




int dynamic_service(const uint8_t operation, const struct ble_gatt_svc_def *svcs, const ble_uuid_t *uuid);

class CGatt // TODO refactor maybe/probably, have to see after introducing auth protocol etc
{
public:
    CGatt();
    ~CGatt();
    CGatt(const CGatt& other) = delete;
    CGatt(CGatt&& other) = default; // TODO need help implementing
    CGatt& operator=(const CGatt& other) = delete;
    CGatt& operator=(CGatt&& other) = default; // TODO need help implementing
public:
    [[nodiscard]] int register_services();
private:
    BleService m_services;
};

}