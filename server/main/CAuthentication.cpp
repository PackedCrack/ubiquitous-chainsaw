#include "CAuthentication.hpp"
#include "../../common/ble_services.hpp"
#include "CGattCharacteristic.hpp"
// std
#include <cstdint>


namespace
{
[[nodiscard]] auto make_signed_mac_callback()
{
	// typedef int ble_gatt_access_fn(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
	return [](uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg) -> int	// type deduction requires exact typematch
	{

		return int32_t{ 0 };
	};
}
[[nodiscard]] ble::CCharacteristic make_signed_mac()
{
	return ble::make_characteristic(ble::ID_CHARS_SIGNED_MAC, make_signed_mac_callback(), ble::CharsPropertyFlag::read);
}
}	// namespace

namespace ble
{
CAuthentication::CAuthentication()
	: m_Characteristics{ make_characteristics() }
	, m_Service{ m_Characteristics }
{}
std::vector<CCharacteristic> CAuthentication::make_characteristics() const
{
	std::vector<CCharacteristic> chars{};
	chars.emplace_back(make_signed_mac());

	return chars;
}
}	// namespace ble