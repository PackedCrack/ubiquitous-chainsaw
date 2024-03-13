#pragma once
#include "../../common/ble_services.hpp"


namespace ble
{
ble_uuid128_t make_ble_uuid_128(uint16_t uniqueValue)
{
  ble_uuid128_t uuid{};
  uuid.u = BLE_UUID_TYPE_128;
  
	static_assert(std::is_trivially_copyable_v<decltype(uuid)>);
  static_assert(std::is_trivially_copyable_v<decltype(BaseUID)>);
	static_assert(ARRAY_SIZE(uuid.value) == sizeof(decltype(BaseUID)));
	std::memcpy(&(m_pNumbleUUID->value[0]), &BaseUID, ARRAY_SIZE(m_pNumbleUUID->value));

  uuid.v[2] = uniqueValue >> 8u;
  uuid.v[3] = uniqueValue & 0x00FF;

	return uuid;
}
}  // namespace ble
