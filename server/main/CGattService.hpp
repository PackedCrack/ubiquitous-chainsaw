#pragma once
#include "CGattCharacteristic.hpp"
// std
#include <vector>


namespace ble
{
class CGattService
{
public:
	
	CGattService(std::vector<CCharacteristic>& characteristics);
	CGattService(const CGattService& other);
	CGattService(CGattService&& other) = default;
	CGattService& operator=(const CGattService& other);
	CGattService& operator=(CGattService&& other) = default;
	explicit operator ble_gatt_svc_def() const;
public:
	[[nodiscard]] const std::vector<ble_gatt_svc_def>& as_nimble_arr() const;
private:
	CGattService() = default;
	[[nodiscard]] CGattService copy(const CGattService& source) const;
	[[nodiscard]] std::vector<ble_gatt_svc_def> make_nimble_svc_arr() const;
private:
	std::unique_ptr<ble_uuid128_t> m_pUUID;
	std::vector<ble_gatt_chr_def> m_Characteristics;
	std::vector<ble_gatt_svc_def> m_Service;
};
}	// namespace ble