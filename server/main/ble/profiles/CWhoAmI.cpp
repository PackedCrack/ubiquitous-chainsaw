#include "CWhoAmI.hpp"
#include "../../../common/ble_services.hpp"
#include "CGattCharacteristic.hpp"
// std
#include <cstdint>
#include <stdexcept>


namespace
{
[[nodiscard]] auto make_callback_signed_mac()
{
	// typedef int ble_gatt_access_fn(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
	return [](uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg) -> int	// type deduction requires exact typematch
	{

		return int32_t{ 0 };
	};
}
[[nodiscard]] ble::CCharacteristic make_characteristic_server_auth()
{
	return ble::make_characteristic(ble::ID_CHARS_SERVER_AUTH, make_callback_signed_mac(), ble::CharsPropertyFlag::read);
}
[[nodiscard]] ble::CCharacteristic make_characteristic_client_auth()
{
	return ble::make_characteristic(ble::ID_CHARS_CLIENT_AUTH, make_callback_signed_mac(), ble::CharsPropertyFlag::read, ble::CharsPropertyFlag::write);
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
Result<CWhoAmI, CWhoAmI::Error> CWhoAmI::make_whoami()
{
	try
	{
		CWhoAmI profile{};
		return Result<CWhoAmI, CWhoAmI::Error>{
			.value = std::make_optional<CWhoAmI>(std::move(profile)),
			.error = Error::none
		};
	}
	catch(const std::runtime_error& err)
	{
		return Result<CWhoAmI, CWhoAmI::Error>{
			.value = std::nullopt,
			.error = Error::outOfHeapMemory
		};
	}
}
CWhoAmI::CWhoAmI()
	: m_Characteristics{ make_characteristics() }
	, m_Service{ m_Characteristics }
{
	int32_t result = ble_gatts_count_cfg(m_Service.as_nimble_arr().data());
    if (result != SUCCESS) 
        throw std::runtime_error{ "nice" };

    result = ble_gatts_add_svcs(m_Service.as_nimble_arr().data());
}

}	// namespace ble