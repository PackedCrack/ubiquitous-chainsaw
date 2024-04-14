#include "CWhoAmI.hpp"
#include "common/ble_services.hpp"
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
	//https://mynewt.apache.org/latest/network/ble_hs/ble_gatts.html?highlight=gatt

	// typedef int ble_gatt_access_fn(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
	return [](uint16_t connHandle, uint16_t attributeHandle, ble_gatt_access_ctxt* pContext) -> int	// type deduction requires exact typematch
	{
		auto operation = ble::CharacteristicAccess{ pContext->op };
		UNHANDLED_CASE_PROTECTION_ON
		switch (operation) 
		{
    	case ble::CharacteristicAccess::read: 
    	{
    	    LOG_INFO("BLE_GATT_ACCESS_OP_READ_CHR");
    	    if (connHandle == BLE_HS_CONN_HANDLE_NONE) // where to put this single #define ?
    	     {
    	        LOG_ERROR("NO CONNECTION EXISTS FOR READ CHARACTERISTIC!");
    	        return static_cast<int32_t>(ble::NimbleErrorCode::noConnection);
    	     }

    	    LOG_INFO_FMT("Characteristic read. conn_handle={} attr_handle={}\n", connHandle, attributeHandle);

			std::vector<uint8_t> writeOperationData {69};
    	    int32_t result = os_mbuf_append(pContext->om,
    	                            writeOperationData.data(),
    	                            writeOperationData.size());
			if (result == static_cast<int32_t>(ble::NimbleErrorCode::success))
			{
				return static_cast<int32_t>(ble::NimbleErrorCode::success);
			}
			else
			{
				return static_cast<int32_t>(ble::NimbleErrorCode::unexpectedCallbackBehavior);
			}   
    	}
    	case ble::CharacteristicAccess::write:
    	{
    	    LOG_INFO("BLE_GATT_ACCESS_OP_WRITE_CHR");
    	    if (pContext->om == nullptr || pContext->om->om_len < 1)
    	    {
    	        LOG_WARN("NO DATA WAS WRITTEN!");
    	        return static_cast<int32_t>(ble::NimbleErrorCode::invalidArguments);
    	    }
    	    uint8_t* pDataBuffer = pContext->om->om_databuf;

    	    // TODO check if this is always the case! 
    	    const uint16_t DATA_OFFSET = 19;
    	    const uint8_t NUM_DATA = *pDataBuffer;
    	    const uint8_t DATA_END = DATA_OFFSET + NUM_DATA;

			// Print Size of data written to which characteristic
			const int MAX_UUID_LEN = 128;
    	    char uuidBuf [MAX_UUID_LEN];
    	    std::string_view charUuid = ble_uuid_to_str((const ble_uuid_t* )&pContext->chr->uuid, uuidBuf);
    	    LOG_INFO_FMT("{} bytes was written to characteristic={}", NUM_DATA, charUuid);

			// Print the written value to terminal
    	    for (int i = 0; i < DATA_END; ++i)
    	    {
    	        std::printf("Data read[%d]: 0x%02x\n", i, pDataBuffer[i]);
    	    }

			// Do some calculation based on the data recieved and write back.
			std::vector<uint8_t> writeOperationData {69};
    	    int result = os_mbuf_append(pContext->om,
    	                            writeOperationData.data(),
    	                            writeOperationData.size());
			if (result == 0)
			{
				return static_cast<int32_t>(ble::NimbleErrorCode::success);
			}
			else
			{
				return static_cast<int32_t>(ble::NimbleErrorCode::unexpectedCallbackBehavior);
			}   
    	}
    	} // switch
		// cppcheck-suppress unknownMacro
		UNHANDLED_CASE_PROTECTION_OFF
    	return static_cast<int32_t>(ble::NimbleErrorCode::unexpectedCallbackBehavior);
	};
}
[[nodiscard]] ble::CCharacteristic make_characteristic_client_auth()
{
	return ble::make_characteristic(ble::ID_CHARACTERISTIC_CLIENT_AUTH, make_callback_client_auth(), ble::CharsPropertyFlag::read, ble::CharsPropertyFlag::write);
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
	return make_characteristic(ID_CHARACTERISTIC_SERVER_AUTH, make_callback_server_auth(pProfile), CharsPropertyFlag::read);
}
}	// namespace ble