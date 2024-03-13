#pragma once
#include "../../common/ble_services.hpp"
#include "../../common/defines.hpp"
// nimble
#include "host/ble_uuid.h"
#include "host/ble_gatt.h"


namespace ble
{
ble_uuid128_t make_ble_uuid128(uint16_t uniqueValue)
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
