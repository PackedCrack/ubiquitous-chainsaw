#include "CGap.hpp"


namespace 
{
uint8_t* make_field_name(const std::string deviceName)
{ return (uint8_t*)deviceName.data(); }

uint8_t make_field_name_len(const std::string deviceName)
{ return static_cast<uint8_t>(deviceName.size()); }

unsigned int make_field_name_is_complete()
{ return 1u; }

uint8_t make_field_flags()
{ return BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP; }

unsigned int make_field_tx_pwr_is_present()
{ return 1u; }

int8_t make_field_pwr_lvl()
{ return BLE_HS_ADV_TX_PWR_LVL_AUTO; }

ble_hs_adv_fields make_advertise_fields(const std::string deviceName) // Nodiscard directive ignored for deviecName??
{
    // i 0% understand what im doing
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"

    // A lot of warnings because missing initilizators
    return ble_hs_adv_fields {
        .flags = make_field_flags(),
        .name = make_field_name(deviceName),
        .name_len = make_field_name_len(deviceName),
        .name_is_complete = make_field_name_is_complete(),
        .tx_pwr_lvl = make_field_pwr_lvl(),
        .tx_pwr_lvl_is_present = make_field_tx_pwr_is_present()
        // todo add services 
    };

     #pragma GCC diagnostic pop
}
ble_gap_adv_params make_advertise_params() 
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
std::optional<ble::CGap::Error> set_adv_fields(const std::string deviceName)
{
	using namespace ble;
	using Error = CGap::Error;

	
    ble_hs_adv_fields fields = make_advertise_fields(deviceName); // only the constructor for ble_hs_adv_fields will be called here
    
    NimbleErrorCode result = NimbleErrorCode{ ble_gap_adv_set_fields(&fields) };
    if (result == NimbleErrorCode::success)
	{
		return std::nullopt;
	}
    else
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
			// make conncetion handle
			// if gap has connection handle
			// then drop
			// else
			// stop advertising
			// query client mac
			// 


            LOG_INFO("BLE_GAP_EVENT_CONNECT");

            pGap->set_connection(event->connect.conn_handle);


            std::optional<CConnectionHandle::Error> result = pGap->discover_services();
            if (result != std::nullopt)
            {
				std::printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
                CConnectionHandle::Error err = *result;
                std::optional<CConnectionHandle::Error> result = pGap->drop_connection(NimbleErrorCode::unexpectedFailure);
                if (err.code != NimbleErrorCode::success)
                {
                    if (err.code == NimbleErrorCode::noConnection)
                        break;

                    if (err.code == NimbleErrorCode::unexpectedFailure)
                        break;
                }
            }
      
            // when connection happens, it is possible to configure another callback that should be used for that connection
            // unable to have locks in here if new procedures are to be created
            break;
        }
        case BLE_GAP_EVENT_DISCONNECT: 
        {
			// if disconnect is connection handle
			// destroy connection handle
			// begin advertisment

            LOG_INFO("BLE_GAP_EVENT_DISCONNECT");
            pGap->reset_connection();

            std::optional<CGap::Error> result = pGap->begin_advertise();
            if (result != std::nullopt)
            {
                CGap::Error err = *result;
                LOG_FATAL_FMT("{}", err.msg);
            }


            //if(result != 0)
            //    LOG_WARN_FMT("ERROR STARTING ADVERTISING! ERROR{}", nimble_error_to_string(result));

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
uint8_t ble_generate_random_device_address() 
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
CGap::CGap() 
    : m_BleAddressType {INVALID_ADDRESS_TYPE} // How to make this better? cant determine bleaddresstype until nimble host stack is started
    , m_Params { make_advertise_params() }
    , m_CurrentConnectionHandle {}
{
	// TODO:
	std::optional<Error> result = start();
	if (result != std::nullopt)
    {
		LOG_FATAL_FMT("{}", result.value().msg.c_str());
    }
}
CGap::~CGap()
{
    std::printf("Gape destructor\n");

	// TODO:
	std::optional<CGap::Error> result = end_advertise();
    if (result != std::nullopt)
    {
        LOG_FATAL_FMT("{}", result.value().msg.c_str());
    }
}
CGap::CGap(CGap&& other) noexcept
    : m_BleAddressType {other.m_BleAddressType}
    , m_Params {std::move(other.m_Params)} 
    , m_CurrentConnectionHandle {std::move(other.m_CurrentConnectionHandle)}
{// no pointers have been moved
}
CGap& CGap::operator=(CGap&& other)
{
    /*
        1. Clean up all visible resources
        2. Transfer the content of other into this
        3. Leave other in a valid but undefined state
    */
    
    // Check if other exists?
    m_BleAddressType = other.m_BleAddressType;
    m_Params = std::move(other.m_Params);
    m_CurrentConnectionHandle = std::move(other.m_CurrentConnectionHandle);
    return *this;
}
void CGap::set_connection(const uint16_t id)
{
    m_CurrentConnectionHandle.set_connection(id);
}
uint16_t CGap::connection_handle() const
{
    return m_CurrentConnectionHandle.handle();
}
/// @brief 
/// @param int32_t reason 
/// @return std::nullopt on success. 
/// NimbleErrorCode::noConnection if there is no connection with the specified handle. 
/// NimbleErrorCode::unknown on failure.
std::optional<CConnectionHandle::Error> CGap::drop_connection(NimbleErrorCode reason)
{
    // if we drop connection manually, ble will start advertising automatically
    //return 
    
    return m_CurrentConnectionHandle.drop(reason);
    //if (result != std::nullopt) 
}
void CGap::reset_connection()
{
    m_CurrentConnectionHandle.reset();

}
/// @brief 
/// @return std::nullopt on success, 
/// NimbleErrorCode::isBusy if advertising is in progress. 
/// NimbleErrorCode::toSmallBuffer if the specified data is too large to fit in an advertisement.
/// NimbleErrorCode::unknown on other failure.
std::optional<CGap::Error> CGap::start()
{
    //ble_svc_gap_init(); // will crash if called before nimble_port_init()
    m_BleAddressType = ble_generate_random_device_address(); // nimble_port_run(); has to be called before this
    ASSERT(m_BleAddressType != INVALID_ADDRESS_TYPE, "Failed to generate a random device address");

    std::string deviceName = "Chainsaw-server";
    int32_t nameSetResult = ble_svc_gap_device_name_set(deviceName.data()); // haven't found which error code is returned when this fails
    if (nameSetResult != static_cast<int32_t>(NimbleErrorCode::success))
    {
          return std::optional<Error> { Error {
			.code = NimbleErrorCode{ nameSetResult },
            .msg = "Unknown error received when setting device name. Return code from esp: " + std::to_string(nameSetResult)
		}};
    }

    std::optional<Error> result = set_adv_fields(deviceName);
    if (result != std::nullopt)
        return result;


    result = begin_advertise();
    if (result != std::nullopt)
        return result;

    return std::nullopt;
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
} // namespace ble
