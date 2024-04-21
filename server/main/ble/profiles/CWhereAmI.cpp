#include "CWhereAmI.hpp"
#include "common/ble_services.hpp"
#include "CGattCharacteristic.hpp"
#include "../../shared/common/ble_services.hpp"
#include "../../shared/common/common.hpp"	
#include "../../server_common.hpp"
#include "../ble_common.hpp"


#include <chrono>
namespace
{

//std::chrono::steady_clock::time_point startTime;
//
//std::chrono::steady_clock::time_point start_timer() {
//    return std::chrono::steady_clock::now();
//}
//double elapsed_time(const std::chrono::steady_clock::time_point& start) {
//    std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
//    std::chrono::duration<double> elapsedTime = endTime - start;
//    return elapsedTime.count(); 
//}

} // namespace


namespace ble
{
CWhereAmI::CWhereAmI()
	: m_Rssi{}
    , m_NotifyHandle{ INVALID_ATTR_HANDLE }
    , m_pPrivateKey{ load_key<security::CEccPrivateKey>(NVS_KEY_SERVER_PRIVATE) }
    , m_pClientPublicKey{ load_key<security::CEccPublicKey>(NVS_KEY_CLIENT_PUBLIC) }
    , m_Characteristics{}
	, m_Service{}
{}
CWhereAmI::CWhereAmI(const CWhereAmI& other)
	: m_Rssi{}
    , m_NotifyHandle{ INVALID_ATTR_HANDLE }
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
    m_NotifyHandle = other.m_NotifyHandle;
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
	chars.emplace_back(make_characteristic_demand_rssi(pProfile));
	chars.emplace_back(make_characteristic_send_rssi(pProfile));

	return chars;
}
auto CWhereAmI::make_callback_demand_rssi(const std::shared_ptr<Profile>& pProfile)
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
                        //startTime = start_timer();

                        LOG_INFO("WHERE_AM_I WRITE EVENT!");
    	                if (pContext->om == nullptr)
    	                {
    	                    LOG_WARN("os_mbuf is invalid!");
    	                    return static_cast<int32_t>(ble::NimbleErrorCode::invalidArguments);
    	                }
                        if (pContext->om->om_len < 1)
    	                {
    	                    LOG_WARN("No data was written to the os_mbuf!");
    	                    return static_cast<int32_t>(ble::NimbleErrorCode::toSmallBuffer);
    	                }

                        uint8_t* pPayloadBuffer = pContext->om->om_databuf;
                        const uint16_t PAYLOAD_OFFSET = 19;
                        const uint8_t PAYLOAD_LEN = *pPayloadBuffer;
                        const uint8_t PAYLOAD_END = PAYLOAD_OFFSET + PAYLOAD_LEN;

                        std::span<uint8_t> packetPayload(pPayloadBuffer + PAYLOAD_OFFSET, PAYLOAD_LEN);
                        
                        //bool verifyResult = pSelf->m_pClientPublicKey->verify_hash(signature, hash);
                        //if (!verifyResult)
                        //{
                        //    LOG_WARN("Was unable to verify the clients signature!");
                        //    return static_cast<int32_t>(ble::NimbleErrorCode::insufficientAuthen);
                        //}

                        // get rssi value
                        int8_t rssiValue {};
                        NimbleErrorCode rssiResult = static_cast<NimbleErrorCode>(ble_gap_conn_rssi(connectionHandle, &rssiValue));
                        if (rssiResult != NimbleErrorCode::success)
                        {
                            LOG_WARN_FMT("Unable to retrieve rssi value. Reason: {}", static_cast<int32_t>(rssiResult));
                            return static_cast<int32_t>(rssiResult);
                        }

                        pSelf->m_Rssi = rssiValue;
                        std::printf("Rssi: %i\n", pSelf->m_Rssi);
                        

                        if (pSelf->m_NotifyHandle == INVALID_ATTR_HANDLE)
                        {
                            auto result = chr_attri_handle(ID_SERVICE_WHEREAMI, ID_CHARACTERISTIC_WHEREAMI_SEND_RSSI);
                            if (!result)
                            {
                                LOG_FATAL("Failed to retrieve a valid chr attribute handle!");
                            }
                            else
                            {
                                pSelf->m_NotifyHandle = result.value();
                            }
                        }

                        std::printf("Handle: %u\n",  pSelf->m_NotifyHandle);
                        
                        //int result = ble_gatts_notify_custom(connectionHandle, pSelf->m_NotifyHandle, NULL);

                        //double elapsedTime = elapsed_time(startTime);
                        //LOG_INFO_FMT("Elapsed time for write callback {}s", elapsedTime);
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
CCharacteristic CWhereAmI::make_characteristic_demand_rssi(const std::shared_ptr<Profile>& pProfile)
{
	return make_characteristic(ID_CHARACTERISTIC_WHEREAMI_DEMAND_RSSI, make_callback_demand_rssi(pProfile), CharsPropertyFlag::write);
}


auto CWhereAmI::make_callback_send_rssi(const std::shared_ptr<Profile>& pProfile)
{
        // cppcheck-suppress constParameterPointer
		return [wpProfile = std::weak_ptr<Profile>{ pProfile }](uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt* pContext) -> int
	{
		std::shared_ptr<Profile> pProfile = wpProfile.lock();
		if(pProfile)
		{
			const CWhereAmI* pSelf = std::get_if<CWhereAmI>(pProfile.get());
			if(pSelf != nullptr)
			{
				auto operation = CharacteristicAccess{ pContext->op };
				switch (operation) 
				{
					case CharacteristicAccess::read:
					{
                        LOG_INFO("WHERE_AM_I NOTIFY EVENT!");


                        //NimbleErrorCode code = append_read_data(pContext->om, rssiValue);
                        //if(code != NimbleErrorCode::success)
                        //{
                        //    LOG_ERROR_FMT("Characteristic callback for Server Auth failed to append its data to the client: \"{}\"", 
                        //                    nimble_error_to_string(code));
                        //}


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
CCharacteristic CWhereAmI::make_characteristic_send_rssi(const std::shared_ptr<Profile>& pProfile)
{
	return make_characteristic(ID_CHARACTERISTIC_WHEREAMI_SEND_RSSI, make_callback_send_rssi(pProfile), CharsPropertyFlag::notify);
}


}	// namespace ble