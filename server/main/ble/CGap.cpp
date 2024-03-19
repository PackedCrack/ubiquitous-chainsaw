#include "CGap.hpp"


namespace 
{
[[nodiscard]] ble_hs_adv_fields make_advertise_fields(const std::string& deviceName) // Nodiscard directive ignored for deviecName??
{
	ASSERT(deviceName.size() <= UINT8_MAX, "Nimble expects devices name to be smaller than 2^8 bytes.");
	static constexpr int32_t FIELD_TX_PWR_PRESENT = 1u;
	static constexpr int32_t FIELD_NAME_COMPLETE = 1u;


	ble_hs_adv_fields fields{};
	fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

	static_assert(alignof(uint8_t) == alignof(std::remove_cvref_t<decltype(*(deviceName.data()))>));
    fields.name = reinterpret_cast<const uint8_t*>(deviceName.data());
    fields.name_len = static_cast<uint8_t>(deviceName.size());
    fields.name_is_complete = FIELD_NAME_COMPLETE;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    fields.tx_pwr_lvl_is_present = FIELD_TX_PWR_PRESENT;

	return fields;
}
[[nodiscard]] ble_gap_adv_params make_default_advertise_params() 
{
    return ble_gap_adv_params { // either initilize all or none warning
		.conn_mode = BLE_GAP_CONN_MODE_UND,
        .disc_mode = BLE_GAP_DISC_MODE_GEN,
        .itvl_min = 0,
        .itvl_max = 0,
        .channel_map = 0,
        .filter_policy = 0u, // whether the advertising packets should be filtered based on the scanner's whitelist (Might be needed for when we scan to retrieve RSSI)
        .high_duty_cycle = 0u //High duty cycle directed advertising increases the frequency of advertising packets sent during directed advertising
    };
}
/// @brief 
/// @param std::string deviceName  
/// @return std::nullopt on success, 
/// NimbleErrorCode::isBusy if advertising is in progress. 
/// NimbleErrorCode::toSmallBuffer if the specified data is too large to fit in an advertisement.
/// NimbleErrorCode::unknown on failure.
[[nodiscard]] std::optional<ble::CGap::Error> set_adv_fields(const std::string& deviceName)
{
	using namespace ble;
	using Error = CGap::Error;

	
    ble_hs_adv_fields fields = make_advertise_fields(deviceName); // only the constructor for ble_hs_adv_fields will be called here
    
    auto result = NimbleErrorCode{ ble_gap_adv_set_fields(&fields) };
    if (result != NimbleErrorCode::success)
	{
		Error err{
			.code = result
		};

		if (result == NimbleErrorCode::isBusy)
    	{
			err.msg = FMT("Advertising is already in progress: \"{}\"", nimble_error_to_string(result));
			return std::make_optional<Error>(std::move(err));
    	}
    	else if (result == NimbleErrorCode::toSmallBuffer)
    	{
			err.msg = FMT("Specified data is too large to fit in an advertisement packet: \"{}\"", nimble_error_to_string(result));
			return std::make_optional<Error>(std::move(err));
    	}
    	else 
    	{
			err.msg = FMT("Unknown error received. Return code from nimble: \"{}\" - \"{}\"", 
							static_cast<int32_t>(result), 
							nimble_error_to_string(result));
			return std::make_optional<Error>(std::move(err));
		}
	}

	return std::nullopt;
}

/////////////////////////////////////////////
// lambda
auto gap_event_handler = [](ble_gap_event* event, void* arg) {
	using namespace ble;
    CGap* pGap = static_cast<CGap*>(arg);

	//UNHANDLED_CASE_PROTECTION_ON
	//switch()
	//{
	//	case CGap::Event::connect:
	//	{
	//		// code
	//		// return 0;
	//	}
	//	case CGap::Event::disconnect:
	//	{
	//		// code
	//		// return 0;
	//	}
	//	case CGap::Event::update:
	//	[[fallthrough]]
	//	case CGap::Event::updateReq:
	//	[[fallthrough]]
	//	case CGap::Event::lastCase:
	//	return UNHANDLED_CASE
	//};
	//UNHANDLED_CASE_PROTECTION_OFF
	//
	// __builtin_unreachable();
    
	switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
        {
            LOG_INFO("BLE_GAP_EVENT_CONNECT");


			CConnection connection{ event->connect.conn_handle };
			if(!pGap->active_connection())
			{
				pGap->set_connection(std::move(connection));

				if(pGap->is_advertising())
				{
					std::optional<CGap::Error> result = pGap->end_advertise();
					if (result)
            		{
						LOG_ERROR_FMT("Gap event callback failed to end advertisment when recieving incoming connection! Reason: \"{}\"",
						 				result->msg);
            		}
				}
			}
			else
			{
            	std::optional<CConnection::Error> result = connection.drop(CConnection::DropCode::busy);
				if(result)
				{
					LOG_ERROR_FMT("Could not drop secondary incoming connection. Reason: \"{}\"", result->msg);
				}
			}
      
            // when connection happens, it is possible to configure another callback that should be used for that connection
            // unable to have locks in here if new procedures are to be created
            break;
        }
        case BLE_GAP_EVENT_DISCONNECT: 
        {
            LOG_INFO("BLE_GAP_EVENT_DISCONNECT");
			
			std::optional<CConnection*> activeCon = pGap->active_connection();
			if(activeCon)
			{
				CConnection* pActiveConnection = *activeCon;
				CConnection connection{ event->disconnect.conn.conn_handle };

				if(*pActiveConnection == connection)
				{
					pGap->set_connection(CConnection{});

					if(!pGap->is_advertising())
					{
						std::optional<CGap::Error> result = pGap->begin_advertise();
            			if (result)
            			{
							LOG_ERROR_FMT(
								"Gap event callback failed to start advertisment when disconnecting previous connection! Reason: \"{}\"",
								 result->msg);
            			}
					}
				}
			}
            break;
        }

        //case BLE_GAP_EVENT_CONN_UPDATE:
        //    LOG_INFO("BLE_GAP_EVENT_CONN_UPDATE");
        //    
        //    break;
        //case BLE_GAP_EVENT_CONN_UPDATE_REQ:
        //    LOG_INFO("BLE_GAP_EVENT_CONN_UPDATE_REQ");
        //    break;
        //case BLE_GAP_EVENT_DISC:
        //    LOG_INFO("BLE_GAP_EVENT_DISC");
        //    break;
        //case BLE_GAP_EVENT_DISC_COMPLETE:
        //   LOG_INFO("BLE_GAP_EVENT_DISC_COMPLETE");
        //   break;
        //case BLE_GAP_EVENT_ADV_COMPLETE:
        //    LOG_INFO("BLE_GAP_EVENT_ADV_COMPLETE");
        //    break;
        //case BLE_GAP_EVENT_ENC_CHANGE:
        //    LOG_INFO("BLE_GAP_EVENT_ENC_CHANGE");
        //    break;
        //case BLE_GAP_EVENT_PASSKEY_ACTION:
        //    LOG_INFO("BLE_GAP_EVENT_PASSKEY_ACTION");
        //    break;
        //case BLE_GAP_EVENT_NOTIFY_RX:
        //    LOG_INFO("BLE_GAP_EVENT_NOTIFY_RX");
        //    break;
        //case BLE_GAP_EVENT_NOTIFY_TX:
        //    LOG_INFO("BLE_GAP_EVENT_NOTIFY_TX");
        //    break;
        //case BLE_GAP_EVENT_SUBSCRIBE:
        //    LOG_INFO("BLE_GAP_EVENT_SUBSCRIBE");
        //    break;
        //case BLE_GAP_EVENT_MTU:
        //    LOG_INFO("BLE_GAP_EVENT_MTU");
        //    break;
        
		// TODO
		default: 
            break;
    } // switch
    return 0;
};
/////////////////////////////////////////////


void print_ble_address()
{
    std::array<uint8_t, 6> bleDeviceAddr {};
    ble::NimbleErrorCode result = ble::NimbleErrorCode{ ble_hs_id_copy_addr(ble::RANDOM_BLUETOOTH_ADDRESS, bleDeviceAddr.data(), nullptr) };

    if (result != ble::NimbleErrorCode::success) 
        LOG_FATAL_FMT("Adress was unable to be retreived {}", ble::nimble_error_to_string(result));

    std::printf("BLE Device Address: %02x:%02x:%02x:%02x:%02x:%02x \n", bleDeviceAddr[5],bleDeviceAddr[4],bleDeviceAddr[3],bleDeviceAddr[2],bleDeviceAddr[1],bleDeviceAddr[0]);
}
[[nodiscard]] uint8_t ble_generate_random_device_address() 
{
    uint8_t addrType = ble::INVALID_ADDRESS_TYPE;
    ble::NimbleErrorCode result = ble::NimbleErrorCode{ ble_hs_util_ensure_addr(ble::RANDOM_BLUETOOTH_ADDRESS) };
    if (result != ble::NimbleErrorCode::success) 
        LOG_FATAL_FMT("No address was able to be ensured ERROR={}", ble::nimble_error_to_string(result));

    result = ble::NimbleErrorCode{ ble_hs_id_infer_auto(ble::PUBLIC_BLUETOOTH_ADDRESS, &addrType) }; // 1/private do not work here, type will depend ble_hs_util_ensure_addr()
    if (result != ble::NimbleErrorCode::success) 
        LOG_FATAL_FMT("No address was able to be inferred ERROR={}", ble::nimble_error_to_string(result));

    ASSERT(addrType == ble::RANDOM_BLUETOOTH_ADDRESS, "Assigned wrong bluetooth address type");

    print_ble_address();

    return addrType;
}
}// namespace

namespace ble
{
void CGap::event_callback_caller(ble_gap_event* pEvent, function eventCallback)
{
	std::function<void(ble_gap_event*)>& cb = *static_cast<std::function<void(ble_gap_event*)>*>(eventCallback);
	cb(pEvent);
}
CGap::CGap() 
    : m_BleAddressType{ ble_generate_random_device_address() } // nimble_port_run(); has to be called before this
    , m_Params{ make_default_advertise_params() }
    , m_ActiveConnection{}
	, m_EventCallback{ /* make cb */}
{
    ASSERT(m_BleAddressType != INVALID_ADDRESS_TYPE, "Failed to generate a random device address");

    std::string deviceName = "Chainsaw-server";
	{
		auto result = NimbleErrorCode{ ble_svc_gap_device_name_set(deviceName.data()) }; // haven't found which error code is returned when this fails
    	if (result != NimbleErrorCode::success)
    	{
			LOG_WARN_FMT("CGap constructor could not set the name of the server. Failed with: \"{}\" - \"{}\"", 
							static_cast<int32_t>(result), 
							nimble_error_to_string(result));
    	}
	}
    
	{
		std::optional<Error> result = set_adv_fields(deviceName);
		if(result)
		{
			if(result->code == NimbleErrorCode::isBusy)
			{
				LOG_WARN_FMT("CGap constructor could not set advertisment fields. Reason: \"{}\"", result->msg);
			}
			else if(result->code == NimbleErrorCode::toSmallBuffer)
			{
				LOG_ERROR_FMT("Failure when trying to set advertisment fields. Reason: \"{}\"", result->msg);
			}
			else
			{
				LOG_FATAL_FMT("Unexpected failure when trying to set advertisment fields. Reason: \"{}\"", result->msg);
			}
		}
	}

	ASSERT(!is_advertising(), "Advertisment was unexpectedly on.");
	std::optional<Error> result = begin_advertise();
    if (result != std::nullopt)
    {
		LOG_FATAL_FMT("CGap constructor could not start the advertising process. Reason: \"{}\"", result->msg);
	}
}
CGap::~CGap()
{
	if(is_advertising())
	{
		std::optional<CGap::Error> result = end_advertise();
    	if (result != std::nullopt)
    	{
			LOG_ERROR_FMT("CGap destructor could not stop an active advertising process. Reason: \"{}\"", result->msg);
    	}
	}
}
CGap::CGap(CGap&& other) noexcept
    : m_BleAddressType{ other.m_BleAddressType }
    , m_Params{ std::move(other.m_Params) } 
    , m_ActiveConnection{ std::move(other.m_ActiveConnection) }
	, m_EventCallback{ std::move(other.m_EventCallback) }
{}
CGap& CGap::operator=(CGap&& other) noexcept
{
	if(this != &other)
	{
		m_BleAddressType = other.m_BleAddressType;
    	m_Params = std::move(other.m_Params);
    	m_ActiveConnection = std::move(other.m_ActiveConnection);
		m_EventCallback = std::move(other.m_EventCallback);
	}

	return *this;
}
void CGap::rssi()
{
    //if(currentConnectionHandle == INVALID_HANDLE_ID)
    //    return;
//
    //int8_t rssiValue {};
    //int rssi = ble_gap_conn_rssi(currentConnectionHandle, &rssiValue);
    //if (rssi != 0)
    //{
    //    LOG_WARN_FMT("Unable to retrieve rssi value: {}", rssi);
    //}
    //else
    //{
    //    LOG_INFO_FMT("RSSI VALUE: {}", rssiValue);
    //}
}
void CGap::set_connection(CConnection&& newConncetion)
{
    m_ActiveConnection = std::move(newConncetion);
}
std::optional<CConnection*> CGap::active_connection()
{
	return m_ActiveConnection ? std::make_optional<CConnection*>(&m_ActiveConnection) : std::nullopt;
}
/// @brief 
/// @param int32_t reason 
/// @return std::nullopt on success. 
/// NimbleErrorCode::noConnection if there is no connection with the specified handle. 
/// NimbleErrorCode::unknown on failure.
std::optional<CConnection::Error> CGap::drop_connection(CConnection::DropCode reason)
{
    // if we drop connection manually, ble will start advertising automatically
    //return 
    
    return m_ActiveConnection.drop(reason);
}
/// @brief 
/// @return std::nullopt on success. NimbleErrorCode::unknown on failure.
std::optional<CGap::Error> CGap::begin_advertise()
{ 
    int32_t result = ble_gap_adv_start(m_BleAddressType, NULL, BLE_HS_FOREVER, &m_Params, gap_event_handler, this); 
    if(result == static_cast<int32_t>(NimbleErrorCode::success))
		return std::nullopt;

    return std::optional<Error> { Error {
			.code = NimbleErrorCode::unexpectedFailure,
            .msg = "Unknown error received when starting advertising. Return code from nimble: " + std::to_string(result)
		}};
}
/// @brief 
/// @return std::nullopt on success. NimbleErrorCode::inProgressOrCompleted if there is no active advertising procedur. NimbleErrorCode::unknown on failure.
std::optional<CGap::Error> CGap::end_advertise()
{ 
	int32_t result = ble_gap_adv_stop();
	if(result == static_cast<int32_t>(NimbleErrorCode::success))
		return std::nullopt;

	if(result == static_cast<int32_t>(NimbleErrorCode::inProgressOrCompleted))
	{
		return std::optional{ Error {
			.code = NimbleErrorCode::inProgressOrCompleted,
			.msg = "no active advertising procedure"			
		}};
	}
	else
	{
		return std::optional<Error> { Error {
			.code = NimbleErrorCode::unexpectedFailure,
            .msg = "Unknown error received. Return code from nimble: " + std::to_string(result)
		}};
	}
}
bool CGap::is_advertising() const
{
	return ble_gap_adv_active();
}
} // namespace ble
