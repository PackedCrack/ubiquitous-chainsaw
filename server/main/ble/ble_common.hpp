#pragma once
#include "../../common/ble_services.hpp"
#include "../../common/defines.hpp"
// nimble
#include "host/ble_uuid.h"
#include "host/ble_gatt.h"


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


inline ble_uuid128_t make_ble_uuid128(uint16_t uniqueValue)
{
	ble_uuid128_t uuid{};
	uuid.u.type = BLE_UUID_TYPE_128;
  
	static_assert(std::is_trivially_copyable_v<decltype(uuid)>);
  	static_assert(std::is_trivially_copyable_v<decltype(BaseUID)>);
	static_assert(ARRAY_SIZE(uuid.value) == sizeof(decltype(BaseUID)));
	std::memcpy(&(uuid.value[0]), &BaseUID, ARRAY_SIZE(uuid.value));

  	uuid.value[2] = uniqueValue >> 8u;
  	uuid.value[3] = uniqueValue & 0x00FF;

	return uuid;
}
}  // namespace ble
