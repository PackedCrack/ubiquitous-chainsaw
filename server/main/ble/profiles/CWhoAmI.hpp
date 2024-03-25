#pragma once
#include "../../../common/SharableThis.hpp"
#include "../../server_common.hpp"
#include "CGattService.hpp"
#include <memory>


namespace ble
{
class CWhoAmI : public SharableThis<CWhoAmI>
{
public:
	CWhoAmI();
	~CWhoAmI() = default;
	CWhoAmI(const CWhoAmI& other);
	CWhoAmI(CWhoAmI&& other) = default;
	CWhoAmI& operator=(const CWhoAmI& other);
	CWhoAmI& operator=(CWhoAmI&& other) noexcept;
public:
	void register_with_nimble();
	[[nodiscard]] ble_gatt_svc_def as_nimble_service() const;
private:
	void retrieve_server_mac();
	[[nodiscard]] std::vector<CCharacteristic> make_characteristics();
	[[nodiscard]] auto make_callback_server_auth();
	[[nodiscard]] CCharacteristic make_characteristic_server_auth();
public:
	std::string m_ServerMac;
	std::string m_ClientMac;
	std::vector<CCharacteristic> m_Characteristics;
	CGattService m_Service;
};
}	// namespace ble