#include "CWhoAmI.hpp"
#include "../../../common/ble_services.hpp"
#include "CGattCharacteristic.hpp"
// std
#include <cstdint>
#include <stdexcept>
#include <array>


namespace
{
[[nodiscard]] auto make_callback_server_auth()
{
	// typedef int ble_gatt_access_fn(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
	return [](uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt) -> int	// type deduction requires exact typematch
	{
		Result<std::string, ble::NimbleErrorCode> result = ble::current_mac_address<std::string>(ble::AddressType::randomMac);
		if(result.value)
		{
			std::printf("\nADDRESS: %s", result.value->c_str());
		}
		

		return int32_t{ 0 };
	};
}
[[nodiscard]] auto make_callback_client_auth()
{
	// typedef int ble_gatt_access_fn(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
	return [](uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt) -> int	// type deduction requires exact typematch
	{
		return int32_t{ 0 };
	};
}
[[nodiscard]] ble::CCharacteristic make_characteristic_server_auth()
{
	return ble::make_characteristic(ble::ID_CHARS_SERVER_AUTH, make_callback_server_auth(), ble::CharsPropertyFlag::read);
}
[[nodiscard]] ble::CCharacteristic make_characteristic_client_auth()
{
	return ble::make_characteristic(ble::ID_CHARS_CLIENT_AUTH, make_callback_client_auth(), ble::CharsPropertyFlag::read, ble::CharsPropertyFlag::write);
}
[[nodiscard]] std::vector<ble::CCharacteristic> make_characteristics()
{
	std::vector<ble::CCharacteristic> chars{};
	chars.emplace_back(make_characteristic_server_auth());
	chars.emplace_back(make_characteristic_client_auth());

	return chars;
}
}	// namespace

namespace ble
{
CWhoAmI::CWhoAmI()
	: m_Characteristics{ make_characteristics() }
	, m_Service{ ID_SERVICE_WHOAMI, m_Characteristics }
{}
ble_gatt_svc_def CWhoAmI::as_nimble_service() const
{
	return static_cast<ble_gatt_svc_def>(m_Service);
}
}	// namespace ble