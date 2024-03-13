#pragma once
#include "CGattCharacteristic.hpp"
// std
#include <vector>


namespace ble
{
class CGattService
{
public:
	CGattService() = default;
	CGattService(std::vector<CCharacteristic>& characteristics);
	CGattService(const CGattService& other);
	CGattService(CGattService&& other) = default;
	CGattService& operator=(const CGattService& other);
	CGattService& operator=(CGattService&& other) = default;
	explicit operator ble_gatt_svc_def() const;
public:
	[[nodiscard]] const std::vector<ble_gatt_svc_def>& as_nimble_arr() const;
private:
	[[nodiscard]] CGattService copy(const CGattService& source) const;
private:
	std::unique_ptr<ble_uuid128_t> m_pUUID;	// Can maybe be placed on stack
	std::vector<ble_gatt_chr_def> m_Characteristics;
	std::vector<ble_gatt_svc_def> m_Service;
};
}	// namespace ble