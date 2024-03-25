#include "CWhoAmI.hpp"
#include "../../../common/ble_services.hpp"
#include "../../server_common.hpp"
#include "CGattCharacteristic.hpp"
// std
#include <cstdint>
#include <stdexcept>
#include <array>



namespace
{
//[[nodiscard]] auto make_callback_server_auth()
//{
//	// typedef int ble_gatt_access_fn(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
//	return [](uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt* pContext) -> int	// type deduction requires exact typematch
//	{
//		Result<std::string, ble::NimbleErrorCode> result = ble::current_mac_address<std::string>(ble::AddressType::randomMac);
//		if(result.value)
//		{
//			std::printf("\nADDRESS: %s", result.value->c_str());
//		}
//		
//
//		return int32_t{ 0 };
//	};
//}
[[nodiscard]] auto make_callback_client_auth()
{
	// typedef int ble_gatt_access_fn(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
	return [](uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt) -> int	// type deduction requires exact typematch
	{
		return int32_t{ 0 };
	};
}
//[[nodiscard]] ble::CCharacteristic make_characteristic_server_auth()
//{
//	return ble::make_characteristic(ble::ID_CHARS_SERVER_AUTH, make_callback_server_auth(), ble::CharsPropertyFlag::read);
//}
[[nodiscard]] ble::CCharacteristic make_characteristic_client_auth()
{
	return ble::make_characteristic(ble::ID_CHARS_CLIENT_AUTH, make_callback_client_auth(), ble::CharsPropertyFlag::read, ble::CharsPropertyFlag::write);
}
//[[nodiscard]] std::vector<ble::CCharacteristic> make_characteristics()
//{
//	std::vector<ble::CCharacteristic> chars{};
//	chars.emplace_back(make_characteristic_server_auth());
//	chars.emplace_back(make_characteristic_client_auth());
//
//	return chars;
//}
}	// namespace

#include <iostream>

namespace ble
{
CWhoAmI::CWhoAmI()
	: m_ServerMac{}
	, m_ClientMac{}
	, m_Characteristics{ make_characteristics() }
	, m_Service{ ID_SERVICE_WHOAMI, m_Characteristics }
{}
CWhoAmI::CWhoAmI(const CWhoAmI& other)
	: m_ServerMac{ other.m_ServerMac }
	, m_ClientMac{ other.m_ClientMac }
	, m_Characteristics{ make_characteristics() }
	, m_Service{ ID_SERVICE_WHOAMI, m_Characteristics }
{}
CWhoAmI& CWhoAmI::operator=(const CWhoAmI& other)
{
	if(this != &other)
	{
		store_this();
		m_ServerMac = other.m_ServerMac;
		m_ClientMac = other.m_ClientMac;
		m_Characteristics = make_characteristics();
		m_Service = CGattService{ ID_SERVICE_WHOAMI, m_Characteristics };
	}

	return *this;
}
CWhoAmI& CWhoAmI::operator=(CWhoAmI&& other) noexcept
{
	if(this != &other)
	{
		*m_pSelf = this;
		m_ServerMac = std::move(other.m_ServerMac);
		m_ClientMac = std::move(other.m_ClientMac);
		m_Characteristics = std::move(other.m_Characteristics);
		m_Service = std::move(other.m_Service);
	}

	return *this;
}
void CWhoAmI::register_with_nimble()
{
	m_Characteristics = make_characteristics();
	m_Service = CGattService{ ID_SERVICE_WHOAMI, m_Characteristics };
}
void CWhoAmI::retrieve_server_mac()
{
	{
		Result<std::string, ble::NimbleErrorCode> result = ble::current_mac_address<std::string>(ble::AddressType::randomMac);
		
		if(result.value)
		{
			m_ServerMac = std::move(result.value.value());
		}
		else
		{
			LOG_WARN("CWhoAmI could not retrieve a random MAC address.. Falling back to public address");

			result = ble::current_mac_address<std::string>(ble::AddressType::publicMac);
			if(result.value)
			{
				m_ServerMac = std::move(*result.value);
			}
			else
			{
				LOG_ERROR("CWhoAmI could not retrieve ANY address!");
			}
		}
	}
	
	
	if(!m_ServerMac.empty())
	{
		// sign mac
	}
}
ble_gatt_svc_def CWhoAmI::as_nimble_service() const
{
	return static_cast<ble_gatt_svc_def>(m_Service);
}
std::vector<CCharacteristic> CWhoAmI::make_characteristics()
{
	std::vector<CCharacteristic> chars{};
	chars.emplace_back(make_characteristic_server_auth());
	chars.emplace_back(make_characteristic_client_auth());

	return chars;
}
auto CWhoAmI::make_callback_server_auth()
{
	return [pWeakThis= share_this(), this](uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt* pContext) -> int	// type deduction requires exact typematch
	{
		auto pThis = pWeakThis.lock();
		if(pThis)
		{
			CWhoAmI* pSelf = *pThis;

			auto operation = CharacteristicAccess{ pContext->op };
			UNHANDLED_CASE_PROTECTION_ON
			switch (operation) 
			{
				case CharacteristicAccess::read:
				{
					if(pSelf->m_ServerMac.empty())
						pSelf->retrieve_server_mac();


					NimbleErrorCode code = append_read_data(pContext->om, pSelf->m_ServerMac);
					if(code != NimbleErrorCode::success)
					{
						LOG_ERROR_FMT("Characteristic callback for Server Auth failed to append its data to the client: \"{}\"", 
										nimble_error_to_string(code));
					}

					print_task_info("nimble_host");
        		    return static_cast<int32_t>(code);
				}
				case CharacteristicAccess::write:
				{
					LOG_ERROR_FMT("Read only Characteristic \"Server Auth\" recieved a Write operation from connection handle: \"{}\"",
									connectionHandle);
				}
        	}
			UNHANDLED_CASE_PROTECTION_OFF
		}
		else
		{
			LOG_WARN("Characteristic callback for \"Server Auth\" failed to take ownership of this pointer storage! It has been deleted.");
		}

		return static_cast<int32_t>(NimbleErrorCode::unexpectedCallbackBehavior);
	};
}
CCharacteristic CWhoAmI::make_characteristic_server_auth()
{
	return make_characteristic(ID_CHARS_SERVER_AUTH, make_callback_server_auth(), CharsPropertyFlag::read);
}
}	// namespace ble