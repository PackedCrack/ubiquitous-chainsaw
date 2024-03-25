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
[[nodiscard]] auto make_callback_client_auth()
{
	// typedef int ble_gatt_access_fn(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
	return [](uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt) -> int	// type deduction requires exact typematch
	{
		return int32_t{ 0 };
	};
}
[[nodiscard]] ble::CCharacteristic make_characteristic_client_auth()
{
	return ble::make_characteristic(ble::ID_CHARS_CLIENT_AUTH, make_callback_client_auth(), ble::CharsPropertyFlag::read, ble::CharsPropertyFlag::write);
}
}	// namespace

namespace ble
{
CWhoAmI::CWhoAmI()
	: m_ServerMac{}
	, m_ClientMac{}
	, m_Characteristics{}
	, m_Service{}
{}
CWhoAmI::CWhoAmI(const CWhoAmI& other)
	: m_ServerMac{ other.m_ServerMac }
	, m_ClientMac{ other.m_ClientMac }
	, m_Characteristics{}
	, m_Service{}
{}
CWhoAmI& CWhoAmI::operator=(const CWhoAmI& other)
{
	if(this != &other)
	{
		m_ServerMac = other.m_ServerMac;
		m_ClientMac = other.m_ClientMac;
		m_Characteristics = std::vector<CCharacteristic>{};
		m_Service = CGattService{};
	}

	return *this;
}
CWhoAmI& CWhoAmI::operator=(CWhoAmI&& other) noexcept
{
	if(this != &other)
	{
		m_ServerMac = std::move(other.m_ServerMac);
		m_ClientMac = std::move(other.m_ClientMac);
		m_Characteristics = std::move(other.m_Characteristics);
		m_Service = std::move(other.m_Service);
	}

	return *this;
}
void CWhoAmI::register_with_nimble(const std::shared_ptr<Profile>& pProfile)
{
	m_Characteristics = make_characteristics(pProfile);
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
std::vector<CCharacteristic> CWhoAmI::make_characteristics(const std::shared_ptr<Profile>& pProfile)
{
	std::vector<CCharacteristic> chars{};
	chars.emplace_back(make_characteristic_server_auth(pProfile));
	chars.emplace_back(make_characteristic_client_auth());

	return chars;
}
auto CWhoAmI::make_callback_server_auth(const std::shared_ptr<Profile>& pProfile)
{
	return [wpProfile = std::weak_ptr<Profile>{ pProfile }](uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt* pContext) -> int	// type deduction requires exact typematch
	{
		std::shared_ptr<Profile> pProfile = wpProfile.lock();
		if(pProfile)
		{
			CWhoAmI* pSelf = std::get_if<CWhoAmI>(pProfile.get());
			if(pSelf != nullptr)
			{
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
				LOG_WARN("Characteristic callback for \"Server Auth\" failed to retrieve pointer to self from shared_ptr to Profile.");
			}
		}
		else
		{
			LOG_WARN("Characteristic callback for \"Server Auth\" failed to take ownership of shared pointer to profile! It has been deleted.");
		}

		return static_cast<int32_t>(NimbleErrorCode::unexpectedCallbackBehavior);
	};
}
CCharacteristic CWhoAmI::make_characteristic_server_auth(const std::shared_ptr<Profile>& pProfile)
{
	return make_characteristic(ID_CHARS_SERVER_AUTH, make_callback_server_auth(pProfile), CharsPropertyFlag::read);
}
}	// namespace ble