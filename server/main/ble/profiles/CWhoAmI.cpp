#include "CWhoAmI.hpp"
#include "common/ble_services.hpp"
#include "../../server_common.hpp"
#include "CGattCharacteristic.hpp"
#include "../../shared/common/ble_services.hpp"
#include "../../shared/common/common.hpp"	

#include "security/CWolfCrypt.hpp"
#include "security/CRandom.hpp"
#include "security/ecc_key.hpp"
#include "security/CHash.hpp"
#include "security/sha.hpp"
#include "../sys/CNonVolatileStorage.hpp"


// std
#include <cstdint>
#include <stdexcept>
#include <array>
#include <cstring> 
#include <type_traits>



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
    	    //uint8_t* pDataBuffer = pContext->om->om_databuf;
//
    	    //// TODO check if this is always the case! 
    	    //const uint16_t DATA_OFFSET = 19;
    	    //const uint8_t NUM_DATA = *pDataBuffer;
    	    //const uint8_t DATA_END = DATA_OFFSET + NUM_DATA;
//
			//// Print Size of data written to which characteristic
			//const int MAX_UUID_LEN = 128;
    	    //char uuidBuf [MAX_UUID_LEN];
    	    //std::string_view charUuid = ble_uuid_to_str((const ble_uuid_t* )&pContext->chr->uuid, uuidBuf);
    	    //LOG_INFO_FMT("{} bytes was written to characteristic={}", NUM_DATA, charUuid);
//
			//// Print the written value to terminal
    	    //for (int i = 0; i < DATA_END; ++i)
    	    //{
    	    //    std::printf("Data read[%d]: 0x%02x\n", i, pDataBuffer[i]);
    	    //}

			// Retrieve the rssi value
			int8_t rssiValue {};
    		int rssiResult = ble_gap_conn_rssi(connHandle, &rssiValue);
    		if (rssiResult != ESP_OK)
    		{
    		    LOG_ERROR("Failed to retrieve RSSI value");
    		}

			//os_mbuf* pDatabuffer1 {};
			//int result = os_mbuf_append(pDatabuffer1,
    	    //                        	&rssiValue,
    	    //                        	sizeof(int8_t));

			std::printf("Rssi value: %d \n", rssiValue);

			//int result = os_mbuf_append(pContext->om,
    	    //                        &rssiValue,
    	    //                        sizeof(int8_t));

			//uint8_t* pDataBuffer = pContext->om->om_databuf;	
			//const uint16_t DATA_OFFSET = 19;
    	    //const uint8_t NUM_DATA = *pDataBuffer;
    	    //const uint8_t DATA_END = DATA_OFFSET + NUM_DATA;
//
    	    //for (int i = 0; i < DATA_END; ++i)
    	    //{
    	    //    std::printf("Data read[%d]: 0x%02x\n", i, pDataBuffer[i]);
    	    //}				

			//std::printf("Result %u\n", result);

			//result = ble_gatts_notify_custom(connHandle, 3u, pContext->om);
		

			//std::printf("Result %u\n", result);

			//result =  ble_gatts_notify(connHandle, 0u);
			// Service attrHandle = 1
			// read only char boob attrhandle = 3
			// read/write char babe attrhandle = 5

			// 1 and 3 works. 5 gets no connectionHandle (5 is the same attrHandle as this char)
			// Which means, we have to retreive the attrHandle of the char we want to use
			
			// 1. retrieve the attribute handle to the correct char
			// 2. get rssi value
			// send notification to that characteristic

			// returns 9 = ble_hs_eapp Application callback behaved unexpectedly. ble_hs_eapp
			// https://mynewt.apache.org/master/network/ble_hs/ble_hs_return_codes.html?highlight=ble_hs_eapp

			//std::vector<uint8_t> writeOperationData {69};
			//int result = os_mbuf_append(pContext->om, writeOperationData.data(), static_cast<uint16_t>(writeOperationData.size()));
			//std::printf("Result %u \n", result);

			//int result = ble_gatts_notify_custom(connHandle, 3u, pContext->om);
			int result = ble_gatts_notify(connHandle, 3u);

			std::printf("Result %u \n", result);


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
	return ble::make_characteristic(ble::ID_CHARACTERISTIC_CLIENT_AUTH, make_callback_client_auth(), ble::CharsPropertyFlag::read, ble::CharsPropertyFlag::write, ble::CharsPropertyFlag::notify);
}
}	// namespace

namespace ble
{
CWhoAmI::CWhoAmI()
	: m_ServerMac{}
	, m_ClientMac{}
	, m_SignedMacData{}
	, m_Characteristics{}
	, m_Service{}
{}
CWhoAmI::CWhoAmI(const CWhoAmI& other)
	: m_ServerMac{ other.m_ServerMac }
	, m_ClientMac{ other.m_ClientMac }
	, m_SignedMacData{ other.m_SignedMacData }
	, m_Characteristics{}
	, m_Service{}
{}
CWhoAmI& CWhoAmI::operator=(const CWhoAmI& other)
{
	if(this != &other)
	{
		m_ServerMac = other.m_ServerMac;
		m_ClientMac = other.m_ClientMac;
		m_SignedMacData = other.m_SignedMacData;
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
		m_SignedMacData = std::move(other.m_SignedMacData);
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
	
	if(!m_ServerMac.empty())
	{

		// TODO: EXTRACT THIS CODE TO sys/server_common.hpp
		std::optional<storage::CNonVolatileStorage::CReader> reader = storage::CNonVolatileStorage::CReader::make_reader(NVS_ENC_NAMESPACE);
		if (!reader.has_value())
		{
			LOG_FATAL("Failed to initilize NVS CReader");
		}
		storage::CNonVolatileStorage::ReadResult<std::vector<uint8_t>> readResult = reader.value().read_binary(NVS_ENC_PRIV_KEY);
		if (readResult.code != storage::NvsErrorCode::success)
		{
			LOG_FATAL("Failed to retrieve the private key");
		}
	
		security::CEccPrivateKey privateKey { std::move(readResult.data.value()) };
		security::CRandom rng = security::CRandom::make_rng().value();
		security::CHash<security::Sha2_256> hash{ m_ServerMac };
		std::vector<security::byte> signature = privateKey.sign_hash(rng, hash);

		size_t packetSize = sizeof(ServerAuthHeader) + hash.size() + signature.size();
		std::vector<security::byte> packetData {common::assert_down_cast<uint8_t>(static_cast<uint8_t>(HashType::Sha2_256)),
												common::assert_down_cast<uint8_t>(sizeof(ServerAuthHeader)),
												common::assert_down_cast<uint8_t>(hash.size()),
												common::assert_down_cast<uint8_t>(sizeof(ServerAuthHeader) + hash.size()),
												common::assert_down_cast<uint8_t>(signature.size())};
		packetData.resize(packetSize);

		// Trust me, it just works :)
		std::memcpy(packetData.data() + sizeof(ServerAuthHeader), hash.data(), hash.size() * sizeof(security::byte));
		std::memcpy((packetData.data() + hash.size() + sizeof(ServerAuthHeader)), signature.data(), signature.size() * sizeof(security::byte));
		
		m_SignedMacData = std::move(packetData);
		if (m_SignedMacData.empty())
		{
			LOG_FATAL("Failed to sign server MAC address!");
		}
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
						//uint8_t* pDataBuffer = pContext->om->om_databuf;	
						//const uint16_t DATA_OFFSET = 19;
    	    			//const uint8_t NUM_DATA = *pDataBuffer;
    	    			//const uint8_t DATA_END = DATA_OFFSET + NUM_DATA;
    	    			//for (int i = 0; i < DATA_END; ++i)
    	    			//{
    	    			//    std::printf("Data read[%d]: 0x%02x\n", i, pDataBuffer[i]);
    	    			//}
						///////////////////////////

						if(pSelf->m_ServerMac.empty())
							pSelf->retrieve_server_mac();

						std::printf("Server MAC: %s \n", pSelf->m_ServerMac.c_str());

						NimbleErrorCode code = append_read_data(pContext->om, pSelf->m_SignedMacData);
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
	return make_characteristic(ID_CHARACTERISTIC_SERVER_AUTH, make_callback_server_auth(pProfile), CharsPropertyFlag::read, CharsPropertyFlag::notify);
}
}	// namespace ble