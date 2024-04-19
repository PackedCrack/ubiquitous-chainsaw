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

            std::printf("ConnectionHandle: %u\n", connectionHandle);


            int8_t rssiValue {};
            int rssiResult = ble_gap_conn_rssi(connectionHandle, &rssiValue);
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
	return ble::make_characteristic(ble::ID_CHARACTERISTIC_CLIENT_NOTIFY, make_callback_client_notify(), ble::CharsPropertyFlag::read, ble::CharsPropertyFlag::notify);
}
}	// namespace

namespace ble
{
CWhereAmI::CWhereAmI()
	: m_Characteristics{}
	, m_Service{}
{}
CWhereAmI::CWhereAmI(const CWhereAmI& other)
	: m_Characteristics{}
	, m_Service{}
{}
CWhereAmI& CWhereAmI::operator=(const CWhereAmI& other)
{
	if(this != &other)
	{
		m_Characteristics = std::vector<CCharacteristic>{};
		m_Service = CGattService{};
	}

	return *this;
}
CWhereAmI& CWhereAmI::operator=(CWhereAmI&& other) noexcept
{
	if(this != &other)
	{
		m_Characteristics = std::move(other.m_Characteristics);
		m_Service = std::move(other.m_Service);
	}

	return *this;
}
void CWhereAmI::register_with_nimble(const std::shared_ptr<Profile>& pProfile)
{
	m_Characteristics = make_characteristics(pProfile);
	m_Service = CGattService{ ID_SERVICE_WWHEREAMI, m_Characteristics };
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
				UNHANDLED_CASE_PROTECTION_ON
				switch (operation) 
				{
					case CharacteristicAccess::read:
					{
                        LOG_ERROR_FMT("Write only Characteristic \"Server Auth\" recieved a Write operation from connection handle: \"{}\"",
										connectionHandle);
					}
					case CharacteristicAccess::write:
					{
                        //LOG_INFO("BLE_GATT_ACCESS_OP_WRITE_CHR");
    	                //if (pContext->om == nullptr || pContext->om->om_len < 1)
    	                //{
    	                //    LOG_WARN("NO DATA WAS WRITTEN!");
    	                //    return static_cast<int32_t>(ble::NimbleErrorCode::invalidArguments);
    	                //}
                        //uint8_t* pDataBuffer = pContext->om->om_databuf;
                        //// TODO check if this is always the case! 
                        //const uint16_t DATA_OFFSET = 19;
                        //const uint8_t NUM_DATA = *pDataBuffer;
                        //const uint8_t DATA_END = DATA_OFFSET + NUM_DATA;
                        //// Print Size of data written to which characteristic
                        //const int MAX_UUID_LEN = 128;
                        //char uuidBuf [MAX_UUID_LEN];
                        //std::string_view charUuid = ble_uuid_to_str((const ble_uuid_t* )&pContext->chr->uuid, uuidBuf);
                        //LOG_INFO_FMT("{} bytes was written to characteristic={}", NUM_DATA, charUuid);
                        //// Print the written value to terminal
                        //for (int i = 0; i < DATA_END; ++i)
                        //{
                        //    std::printf("Data read[%d]: 0x%02x\n", i, pDataBuffer[i]);
                        //}
	
                        std::printf("oohh, write me AGAIN!\n");
                        std::printf("ConnectionHandle: %u\n", connectionHandle);
                        
                        int result = ble_gatts_notify_custom(connectionHandle, 3u, NULL);
                        std::printf("ble_gatts_notify() Result: %u\n", result);
                        return static_cast<int32_t>(ble::NimbleErrorCode::success);
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
CCharacteristic CWhereAmI::make_characteristic_client_query(const std::shared_ptr<Profile>& pProfile)
{
	return make_characteristic(ID_CHARACTERISTIC_CLIENT_QUERY, make_callback_client_query(pProfile), CharsPropertyFlag::write);
}
}	// namespace ble