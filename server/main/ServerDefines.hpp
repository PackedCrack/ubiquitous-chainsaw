#pragma once

/* STD */
#include <vector>
/* BLE */
#include "host/ble_uuid.h"




namespace ble
{

constexpr uint16_t INVALID_HANDLE_ID = 65535u;
constexpr uint8_t INVALID_ADDRESS_TYPE = 255u;
constexpr int MAX_UUID_LEN = 128;

constexpr int SUCCESS = 0;
constexpr int FAIL = 1;
constexpr int PROCEDURE_HAS_FINISHED = 14;

struct BleCharacteristic
{
    ble_uuid_any_t uuid;
    uint16_t handle;
    uint16_t handleValue;
    uint8_t properties;
};

struct BleService 
{
    ble_uuid_any_t uuid;
    uint16_t connHandle;
    uint16_t handleStart;
    uint16_t handleEnd;
    std::vector<BleCharacteristic> characteristics;
};

} // namespace ble
