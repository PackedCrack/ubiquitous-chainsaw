#include "CGap.hpp"


namespace 
{
[[nodiscard]] std::string gap_event_to_str(ble::CGap::Event evnt)
{
	UNHANDLED_CASE_PROTECTION_ON
	switch(evnt)
	{
		case ble::CGap::Event::connect: return "connect";
		case ble::CGap::Event::disconnect: return "disconnect";
		case ble::CGap::Event::update: return "update";
		case ble::CGap::Event::updateReq: return "updateReq";
		case ble::CGap::Event::l2capUpdateReq: return "l2capUpdateReq";
		case ble::CGap::Event::termFailure: return "termFailure";
		case ble::CGap::Event::disc: return "disc";
		case ble::CGap::Event::discComplete: return "discComplete";
		case ble::CGap::Event::advertismentComplete: return "advertismentComplete";
		case ble::CGap::Event::encryptionChanged: return "encryptionChanged";
		case ble::CGap::Event::passkeyAction: return "passkeyAction";
		case ble::CGap::Event::notifyRecieve: return "notifyRecieve";
		case ble::CGap::Event::notifyTransfer: return "notifyTransfer";
		case ble::CGap::Event::subscribe: return "subscribe";
		case ble::CGap::Event::mtu: return "mtu";
		case ble::CGap::Event::identityResolved: return "identityResolved";
		case ble::CGap::Event::repeatPairing: return "repeatPairing";
		case ble::CGap::Event::physicalUpdateComplete: return "physicalUpdateComplete";
		case ble::CGap::Event::extDisc: return "";
		case ble::CGap::Event::periodicSync: return "periodicSync";
		case ble::CGap::Event::periodicSyncReport: return "periodicSyncReport";
		case ble::CGap::Event::periodicSyncLost: return "periodicSyncLost";
		case ble::CGap::Event::scanRequireRCVD: return "scanRequireRCVD";
		case ble::CGap::Event::periodicTransfer: return "periodicTransfer";
		case ble::CGap::Event::pathlossThreshold: return "pathlossThreshold";
		case ble::CGap::Event::transmitPower: return "transmitPower";
		case ble::CGap::Event::subrateChange: return "subrateChange";
	};
	UNHANDLED_CASE_PROTECTION_OFF
	
	 __builtin_unreachable();
}
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
/// NimbleErrorCode::unexpectedFailure on failure.
[[nodiscard]] std::optional<ble::NimbleErrorCode> set_adv_fields(const std::string& deviceName)
{
    ble_hs_adv_fields fields = make_advertise_fields(deviceName); // only the constructor for ble_hs_adv_fields will be called here
    
    auto result = ble::NimbleErrorCode{ ble_gap_adv_set_fields(&fields) };
    if (result != ble::NimbleErrorCode::success)
	{
		if (result == ble::NimbleErrorCode::isBusy)
    	{
			return std::make_optional<ble::NimbleErrorCode>(result);
    	}
    	else if (result == ble::NimbleErrorCode::toSmallBuffer)
    	{
			return std::make_optional<ble::NimbleErrorCode>(result);
    	}
    	else 
    	{
			LOG_WARN_FMT("Unknown error received. Return code from nimble: \"{}\" - \"{}\"",
							static_cast<int32_t>(result), 
							nimble_error_to_string(result));
			return std::make_optional<ble::NimbleErrorCode>(ble::NimbleErrorCode::unexpectedFailure);
		}
	}

	return std::nullopt;
}
void print_ble_address()
{
    std::array<uint8_t, 6> bleDeviceAddr {};
    ble::NimbleErrorCode result = ble::NimbleErrorCode{ 
		ble_hs_id_copy_addr(static_cast<uint8_t>(ble::AddressType::randomMac), bleDeviceAddr.data(), nullptr) };

    if (result != ble::NimbleErrorCode::success) 
        LOG_FATAL_FMT("Adress was unable to be retreived {}", ble::nimble_error_to_string(result));

    std::printf("BLE Device Address: %02x:%02x:%02x:%02x:%02x:%02x \n", 
				bleDeviceAddr[5], 
				bleDeviceAddr[4],
				bleDeviceAddr[3],
				bleDeviceAddr[2],
				bleDeviceAddr[1],
				bleDeviceAddr[0]);
}
[[nodiscard]] ble::AddressType generate_random_device_address() 
{
	static constexpr bool PREFER_RANDOM = true;
	static constexpr bool USE_PRIVATE_ADDR = false;

    uint8_t expectedAddrType = ble::INVALID_ADDRESS_TYPE;
    auto result = ble::NimbleErrorCode{ ble_hs_util_ensure_addr(PREFER_RANDOM) };
    if (result != ble::NimbleErrorCode::success)
	{
		LOG_FATAL_FMT("No address was able to be ensured ERROR={}", ble::nimble_error_to_string(result));
	}
        
    result = ble::NimbleErrorCode{ ble_hs_id_infer_auto(USE_PRIVATE_ADDR, &expectedAddrType) }; // 1/private do not work here, type will depend ble_hs_util_ensure_addr()
    if (result != ble::NimbleErrorCode::success)
	{
		LOG_FATAL_FMT("No address was able to be inferred ERROR={}", ble::nimble_error_to_string(result));
	}

    ASSERT(expectedAddrType == static_cast<uint8_t>(ble::AddressType::randomMac), "Assigned wrong bluetooth address type");
	#ifndef NDEBUG
    print_ble_address();
	#endif

    return ble::AddressType{ expectedAddrType };
}
}// namespace

namespace ble
{
int CGap::event_callback_caller(ble_gap_event* pEvent, void* eventCallback)
{
	const std::function<void(ble_gap_event*)>& cb = *static_cast<std::function<void(ble_gap_event*)>*>(eventCallback);
	cb(pEvent);

	return 0;
}
CGap::CGap() 
    : m_BleAddressType{ generate_random_device_address() } // nimble_port_run(); has to be called before this
    , m_Params{ make_default_advertise_params() }
    , m_ActiveConnection{}
	, m_EventCallback{ make_event_callback() }
{
    ASSERT(m_BleAddressType != AddressType::invalid, "Failed to generate a random device address");

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
		std::optional<NimbleErrorCode> error = set_adv_fields(deviceName);
		if(error)
		{
			if(error == NimbleErrorCode::isBusy)
			{
				LOG_WARN_FMT("CGap constructor could not set advertisment fields. Because Advertising is already in progress: \"{}\"",
								nimble_error_to_string(*error));
			}
			else if(error == NimbleErrorCode::toSmallBuffer)
			{
				LOG_ERROR_FMT("Failure when trying to set advertisment fields. Because Advertising is already in progress: \"{}\"",
								nimble_error_to_string(*error));
			}
			else if(error == NimbleErrorCode::unexpectedFailure)
			{
				LOG_FATAL_FMT("Unexpected failure when trying to set advertisment fields. ErrorCode: \"{}\"", 
								nimble_error_to_string(*error));
			}
			else
			{
				ASSERT(false, "set_adv_fields returned an undocumented error code.");
			}
		}
	}

	ASSERT(!is_advertising(), "Advertisment was unexpectedly on.");
	std::optional<Error> error = begin_advertise();
    if (error)
    {
		LOG_FATAL_FMT("CGap constructor could not start the advertising process. Reason: \"{}\"", error->msg);
	}
}
CGap::~CGap()
{
	if(is_advertising())
	{
		std::optional<CGap::Error> error = end_advertise();
    	if (error)
    	{
			LOG_ERROR_FMT("CGap destructor could not stop an active advertising process. Reason: \"{}\"", error->msg);
    	}
	}
}
CGap::CGap(CGap&& other) noexcept
    : m_BleAddressType{ other.m_BleAddressType }
    , m_Params{ std::move(other.m_Params) } 
    , m_ActiveConnection{ std::move(other.m_ActiveConnection) }
	, m_EventCallback{ make_event_callback() }
{}
CGap& CGap::operator=(CGap&& other) noexcept
{
	if(this != &other)
	{
		m_BleAddressType = other.m_BleAddressType;
    	m_Params = std::move(other.m_Params);
    	m_ActiveConnection = std::move(other.m_ActiveConnection);
		m_EventCallback = make_event_callback();
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
    int32_t result = ble_gap_adv_start(
		static_cast<uint8_t>(m_BleAddressType), nullptr, BLE_HS_FOREVER, &m_Params, CGap::event_callback_caller, &m_EventCallback);
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
std::function<void(ble_gap_event*)> CGap::make_event_callback()
{
	return [this](ble_gap_event* pEvent)
	{
		auto evnt = CGap::Event{ pEvent->type };
		UNHANDLED_CASE_PROTECTION_ON
		switch(evnt)
		{
			case CGap::Event::connect:
			{
				LOG_INFO("BLE_GAP_EVENT_CONNECT");


				CConnection connection{ pEvent->connect.conn_handle };
				if(!this->active_connection())
				{
					this->set_connection(std::move(connection));

					if(this->is_advertising())
					{
						std::optional<CGap::Error> result = this->end_advertise();
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
            	return;
			}
			case CGap::Event::disconnect:
			{
				LOG_INFO("BLE_GAP_EVENT_DISCONNECT");
			
				std::optional<CConnection*> activeCon = this->active_connection();
				if(activeCon)
				{
					CConnection* pActiveConnection = *activeCon;
					CConnection connection{ pEvent->disconnect.conn.conn_handle };

					if(*pActiveConnection == connection)
					{
						this->set_connection(CConnection{});

						if(!this->is_advertising())
						{
							std::optional<CGap::Error> result = this->begin_advertise();
            				if (result)
            				{
								LOG_ERROR_FMT(
									"Gap event callback failed to start advertisment when disconnecting previous connection! Reason: \"{}\"",
									 result->msg);
            				}
						}
					}
				}
            	return;
			}
			case CGap::Event::update:
			[[fallthrough]];
			case CGap::Event::updateReq:
			[[fallthrough]];
			case ble::CGap::Event::l2capUpdateReq: 
			[[fallthrough]];
			case ble::CGap::Event::termFailure: 
			[[fallthrough]];
			case ble::CGap::Event::disc: 
			[[fallthrough]];
			case ble::CGap::Event::discComplete: 
			[[fallthrough]];
			case ble::CGap::Event::advertismentComplete: 
			[[fallthrough]];
			case ble::CGap::Event::encryptionChanged: 
			[[fallthrough]];
			case ble::CGap::Event::passkeyAction:
			[[fallthrough]];
			case ble::CGap::Event::notifyRecieve: 
			[[fallthrough]];
			case ble::CGap::Event::notifyTransfer: 
			[[fallthrough]];
			case ble::CGap::Event::subscribe: 
			[[fallthrough]];
			case ble::CGap::Event::mtu: 
			[[fallthrough]];
			case ble::CGap::Event::identityResolved: 
			[[fallthrough]];
			case ble::CGap::Event::repeatPairing: 
			[[fallthrough]];
			case ble::CGap::Event::physicalUpdateComplete: 
			[[fallthrough]];
			case ble::CGap::Event::extDisc: 
			[[fallthrough]];
			case ble::CGap::Event::periodicSync: 
			[[fallthrough]];
			case ble::CGap::Event::periodicSyncReport:
			[[fallthrough]];
			case ble::CGap::Event::periodicSyncLost: 
			[[fallthrough]];
			case ble::CGap::Event::scanRequireRCVD: 
			[[fallthrough]];
			case ble::CGap::Event::periodicTransfer: 
			[[fallthrough]];
			case ble::CGap::Event::pathlossThreshold: 
			[[fallthrough]];
			case ble::CGap::Event::transmitPower: 
			[[fallthrough]];
			case ble::CGap::Event::subrateChange:
			{
				LOG_WARN_FMT("Unhandled Gap Event: \"{}\"", gap_event_to_str(evnt));
				return;
			}
		};
		UNHANDLED_CASE_PROTECTION_OFF

		LOG_ERROR_FMT("Unknown gap event: \"{}\"!", pEvent->type);
	};
}
} // namespace ble
