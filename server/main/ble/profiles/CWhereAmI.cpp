#include "CWhereAmI.hpp"
#include "common/ble_services.hpp"
#include "CGattCharacteristic.hpp"
#include "../../shared/common/ble_services.hpp"
#include "../../shared/common/common.hpp"	
#include "../../server_common.hpp"

// std

namespace
{
[[nodiscard]] auto make_callback_client_notify()
{
	// typedef int ble_gatt_access_fn(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
	return [](uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt* pContext) -> int	// type deduction requires exact typematch
	{
		auto operation = ble::CharacteristicAccess{ pContext->op };
		UNHANDLED_CASE_PROTECTION_ON
		switch (operation) 
		{
    	case ble::CharacteristicAccess::read: 
    	{
    	    std::printf("aah, notify me AGAIN!\n");

            std::printf("ConnectionHandle: %u\n", connectionHandle); // is 65535 here


            int8_t rssiValue {};
            int rssiResult = ble_gap_conn_rssi(1u, &rssiValue);
            if (rssiResult != ESP_OK)
            {
                LOG_ERROR("Failed to retrieve RSSI value");
            }
            std::printf("RSSI result %u \n", rssiResult);
            std::printf("Rssi value: %d \n", rssiValue);


            return static_cast<int32_t>(ble::NimbleErrorCode::success);
    	}
    	case ble::CharacteristicAccess::write:
    	{
            LOG_ERROR_FMT("Notify only Characteristic \"Server Auth\" recieved a Write operation from connection handle: \"{}\"",
										connectionHandle);
    	}
    	} // switch
		// cppcheck-suppress unknownMacro
		UNHANDLED_CASE_PROTECTION_OFF
    	return static_cast<int32_t>(ble::NimbleErrorCode::unexpectedCallbackBehavior);
	};
}   
[[nodiscard]] ble::CCharacteristic make_characteristic_client_notify()
{
	return ble::make_characteristic(ble::ID_CHARACTERISTIC_WHEREAMI_SEND_RSSI, make_callback_client_notify(), ble::CharsPropertyFlag::read, ble::CharsPropertyFlag::notify);
}
}	// namespace

namespace ble
{
CWhereAmI::CWhereAmI()
	: m_Rssi{}
    , m_pPrivateKey{ load_key<security::CEccPrivateKey>(NVS_KEY_SERVER_PRIVATE) }
    , m_pClientPublicKey{ load_key<security::CEccPublicKey>(NVS_KEY_CLIENT_PUBLIC) }
    , m_Characteristics{}
	, m_Service{}
{}
CWhereAmI::CWhereAmI(const CWhereAmI& other)
	: m_Rssi{}
    , m_pPrivateKey{ nullptr }
    , m_pClientPublicKey{ nullptr } 
    , m_Characteristics{}
	, m_Service{}
{
    copy(other);
}
CWhereAmI& CWhereAmI::operator=(const CWhereAmI& other)
{
	if(this != &other)
	{
        copy(other);
	}

	return *this;
}
void CWhereAmI::copy(const CWhereAmI& other)
{
    m_Rssi = other.m_Rssi;
    m_pPrivateKey =  load_key<security::CEccPrivateKey>(NVS_KEY_SERVER_PRIVATE);
    m_pClientPublicKey = load_key<security::CEccPublicKey>(NVS_KEY_CLIENT_PUBLIC);
	m_Characteristics = std::vector<CCharacteristic>{};
    m_Service = CGattService{};
}
void CWhereAmI::register_with_nimble(const std::shared_ptr<Profile>& pProfile)
{
	m_Characteristics = make_characteristics(pProfile);
	m_Service = CGattService{ ID_SERVICE_WHEREAMI, m_Characteristics };
}
ble_gatt_svc_def CWhereAmI::as_nimble_service() const
{
	return static_cast<ble_gatt_svc_def>(m_Service);
}
std::vector<CCharacteristic> CWhereAmI::make_characteristics(const std::shared_ptr<Profile>& pProfile)
{
	std::vector<CCharacteristic> chars{};
	chars.emplace_back(make_characteristic_client_notify());
	chars.emplace_back(make_characteristic_client_query(pProfile));

	return chars;
}
auto CWhereAmI::make_callback_client_query(const std::shared_ptr<Profile>& pProfile)
{
		return [wpProfile = std::weak_ptr<Profile>{ pProfile }](uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt* pContext) -> int
	{
		std::shared_ptr<Profile> pProfile = wpProfile.lock();
		if(pProfile)
		{
			CWhereAmI* pSelf = std::get_if<CWhereAmI>(pProfile.get());
			if(pSelf != nullptr)
			{
				auto operation = CharacteristicAccess{ pContext->op };
				switch (operation) 
				{
					case CharacteristicAccess::write:
					{
                        LOG_INFO("WHEREAMI write callback event!");
    	                if (pContext->om == nullptr || pContext->om->om_len < 1)
    	                {
    	                    LOG_WARN("NO DATA WAS WRITTEN!");
    	                    return static_cast<int32_t>(ble::NimbleErrorCode::invalidArguments);
    	                }

                        uint8_t* pPayloadBuffer = pContext->om->om_databuf;
                        const uint16_t PAYLOAD_OFFSET = 19;
                        const uint8_t PAYLOAD_LEN = *pPayloadBuffer;
                        const uint8_t PAYLOAD_END = PAYLOAD_OFFSET + PAYLOAD_LEN;

                        std::span<uint8_t> packetPayload(pPayloadBuffer + PAYLOAD_OFFSET, PAYLOAD_LEN);

                        std::printf("Received payload: \n");
                        for (auto&& bite : packetPayload) 
                        {
                            std::printf("0x%02x\n", bite);
                        }

                        //const int MAX_UUID_LEN = 128;
                        //char uuidBuf [MAX_UUID_LEN];
                        //std::string_view charUuid = ble_uuid_to_str((const ble_uuid_t* )&pContext->chr->uuid, uuidBuf);
                        //LOG_INFO_FMT("{} bytes was written to characteristic={}", DATA_LEN, charUuid);
            
                        //int result = ble_gatts_notify_custom(connectionHandle, 3u, NULL);
                        return static_cast<int32_t>(ble::NimbleErrorCode::success);
					}
        		}
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
CCharacteristic CWhereAmI::make_characteristic_client_query(const std::shared_ptr<Profile>& pProfile)
{
	return make_characteristic(ID_CHARACTERISTIC_WHEREAMI_DEMAND_RSSI, make_callback_client_query(pProfile), CharsPropertyFlag::write);
}
}	// namespace ble