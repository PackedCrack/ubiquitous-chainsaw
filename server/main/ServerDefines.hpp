#pragma once


/* STD */
#include <inttypes.h>

/* BLE */
#include "host/ble_hs.h"
//#include "host/ble_uuid.h"




namespace ble
{

// GAP DEFINES
constexpr uint16_t INVALID_HANDLE_ID = 65535u;
constexpr uint8_t INVALID_ADDRESS_TYPE = 255u;
constexpr int RANDOM_BLUETOOTH_ADDRESS = 1;
constexpr int PUBLIC_BLUETOOTH_ADDRESS = 0;
constexpr int MAX_UUID_LEN = 128;

constexpr int SUCCESS = 0;
constexpr int FAIL = 1;
constexpr int PROCEDURE_HAS_FINISHED = 14;
constexpr int UNRECOVERABLE_ERROR = 98;
constexpr int RECOVERABLE_ERROR = 99;

// GATT DEFINES
constexpr int DATA_END = 0;
constexpr int NUM_SERVICES = 1;
constexpr int SERVICE_SIZE = 2; // name?



} // namespace ble
