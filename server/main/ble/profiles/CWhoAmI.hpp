#pragma once
#include "../../server_common.hpp"
#include "CGattService.hpp"


namespace ble
{
class CWhoAmI
{
public:
	CWhoAmI();
	~CWhoAmI() = default;
	CWhoAmI(const CWhoAmI& other) = default;
	CWhoAmI(CWhoAmI&& other) = default;
	CWhoAmI& operator=(const CWhoAmI& other) = default;
	CWhoAmI& operator=(CWhoAmI&& other) = default;
public:
	[[nodiscard]] ble_gatt_svc_def as_nimble_service() const;
private:
	std::vector<CCharacteristic> m_Characteristics;
	CGattService m_Service;
};
}	// namespace ble