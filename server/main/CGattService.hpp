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
private:
	CGattService() = default;
	[[nodiscard]] CGattService copy(const CGattService& source) const;
private:
	std::unique_ptr<ble_uuid128_t> m_pUUID;
	std::vector<ble_gatt_chr_def> m_Characteristics;
};
}	// namespace ble