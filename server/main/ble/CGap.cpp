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
//auto  descriptor_discovery_event_handler = [](uint16_t conn_handle,
//                                            const struct ble_gatt_error *error,
//                                            uint16_t chr_val_handle,
//                                            const struct ble_gatt_dsc *dsc,
//                                            void *arg) {
//    int result = error->status;
//    if (result != ESP_OK) 
//    {
//        LOG_INFO_FMT("Error during descriptor discovery: {}", result);
//        return 0;
//    } 
//
//    char uuidBuf [128];
//    std::string uuid = ble_uuid_to_str((const ble_uuid_t* )&dsc->uuid, uuidBuf);
//    LOG_INFO_FMT("Descriptor discovered: handle={}, UUID={}", dsc->handle, uuid);
//    return 0;
//};
auto characteristic_discovery_event_handler = [](uint16_t connHandle,
                                            const struct ble_gatt_error *error,
                                            const struct ble_gatt_chr *characteristic,
                                            void *arg) {

    // https://mynewt.apache.org/master/network/ble_hs/ble_hs_return_codes.html#return-codes-att    

    ble::CConnectionHandle* pConnHandle = static_cast<ble::CConnectionHandle*>(arg);
    ASSERT(pConnHandle->handle() == connHandle, "Error casting void* to CConnectionHandle");

    int returnedResult = error->status;

    if (returnedResult == BLE_HS_EINVAL)
    {
        // Need to double check using other clients that this is actually the case. 
        //From what i can see, i get no specific return value that indicated a finished discovery. it always ends with 3.
        LOG_INFO("Characteristic discovery has finished");
        return BLE_HS_EINVAL;
    }


    if (returnedResult == BLE_HS_EBADDATA)
    {
        LOG_INFO_FMT("Unrecognized Characteristic found: {}", returnedResult);
        return BLE_HS_EBADDATA;
        //Message: Characteristic: UUID=0x2b3a handle=4 handleValue=5 properties=2                                                                                    
    }


    if (returnedResult != ble::SUCCESS)
    {
        LOG_ERROR_FMT("Characteristic discovery error: {}", returnedResult);
        return BLE_HS_EUNKNOWN;
    }


    /*  PROPERTIES!!!
#define BLE_GATT_CHR_PROP_BROADCAST                     0x01
#define BLE_GATT_CHR_PROP_READ                          0x02
#define BLE_GATT_CHR_PROP_WRITE_NO_RSP                  0x04
#define BLE_GATT_CHR_PROP_WRITE                         0x08
#define BLE_GATT_CHR_PROP_NOTIFY                        0x10
#define BLE_GATT_CHR_PROP_INDICATE                      0x20
#define BLE_GATT_CHR_PROP_AUTH_SIGN_WRITE               0x40
#define BLE_GATT_CHR_PROP_EXTENDED                      0x80
    */

    // find the correct service and add the characteristic
    for (auto& service : pConnHandle->services())
    {
        if (characteristic->def_handle > service.handleStart && characteristic->def_handle < service.handleEnd)
        {
            service.characteristics.emplace_back(
                ble::BleClientCharacteristic {
                    .uuid = characteristic->uuid,
                    .handle = characteristic->def_handle,
                    .handleValue = characteristic->val_handle,
                    .properties = characteristic->properties
                }
            );

            // print to terminal only
            const uint8_t CHARACTERISTIC_INDEX = (service.characteristics.size() - 1);
            const uint16_t CHARACTERISTIC_HANDLE = service.characteristics[ CHARACTERISTIC_INDEX ].handle;
            const uint16_t HANDLE_VALUE = service.characteristics[ CHARACTERISTIC_INDEX ].handleValue;

            char serviceUuidBuf [ble::MAX_UUID_LEN];
            std::string characteristicUuidToString = ble_uuid_to_str(reinterpret_cast<const ble_uuid_t*>( &service.characteristics[ CHARACTERISTIC_INDEX ].uuid ), serviceUuidBuf);
            LOG_INFO_FMT("Added New Characteristic: UUID={} handle={} handleValue={}", characteristicUuidToString, CHARACTERISTIC_HANDLE, HANDLE_VALUE);
            break;
        }
    }

    return ble::SUCCESS;
};
int discover_service_characteristics(const uint16_t connhandle, const uint16_t handleStart, const uint16_t handleEnd, ble::CConnectionHandle& connHandler)
{
    return ble_gattc_disc_all_chrs(connhandle, handleStart, handleEnd, characteristic_discovery_event_handler, &connHandler);
}
auto service_discovery_event_handler = [](uint16_t connHandle,
                    const struct ble_gatt_error *error,
                    const struct ble_gatt_svc *service,
                    void *arg) {
	using namespace ble;


    // https://mynewt.apache.org/v1_8_0/network/ble_hs/ble_hs_return_codes.html
    CConnectionHandle* pConnHandle = static_cast<ble::CConnectionHandle*>(arg);
    ASSERT(pConnHandle->handle() == connHandle, "Error casting void* to CConnectionHandle");
    

    uint16_t returnedResult = error->status;

    if (returnedResult != PROCEDURE_HAS_FINISHED && returnedResult != SUCCESS)
    {
        LOG_ERROR_FMT("Service Discovery Process error: {}", returnedResult);
        return BLE_HS_EUNKNOWN; // Unexpected failure; catch all.
    }


    if (returnedResult == PROCEDURE_HAS_FINISHED)
    {
        LOG_INFO("Discovery procedure has finished!");
        return PROCEDURE_HAS_FINISHED;
    }


    if (returnedResult == SUCCESS)
    {
        pConnHandle->add_service(
            BleClientService {
                    .uuid = service->uuid,
                    .connHandle = connHandle,
                    .handleStart = service->start_handle,
                    .handleEnd = service->end_handle,
                    .characteristics {}
            }
        );

        const uint8_t SERVICE_INDEX = (pConnHandle->num_services() - 1);
        const uint16_t HANDLE_START = pConnHandle->services()[ SERVICE_INDEX ].handleStart;
        const uint16_t HANDLE_END = pConnHandle->services()[ SERVICE_INDEX ].handleEnd;

        // printing to terminal only
        char serviceUuidBuf [MAX_UUID_LEN];
        std::string serviceUuidToString = ble_uuid_to_str(reinterpret_cast<const ble_uuid_t*>( &pConnHandle->services()[ SERVICE_INDEX ].uuid ), serviceUuidBuf);
        LOG_INFO_FMT("Added New Service: UUID={} handle={} start={} end={}", serviceUuidToString, pConnHandle->handle(), HANDLE_START, HANDLE_END);

        //discover_service_characteristics(&pConnHandle, service->start_handle, service->end_handle);
        NimbleErrorCode result = NimbleErrorCode{ discover_service_characteristics(pConnHandle->handle(), HANDLE_START, HANDLE_END, *pConnHandle) };
        if (result != NimbleErrorCode::success) 
        {
            LOG_WARN_FMT("Failed to initiate characteristic discovery. Error: {}", ble::nimble_error_to_string(result));
            // How to handle? drop existing connection?
            return BLE_HS_EUNKNOWN;
        }

        return SUCCESS;
    }

    ASSERT(0, "End of service_discovery_event_handler() reached!");
    return BLE_HS_EUNKNOWN; // should not be triggered
};
/// @brief 
/// @param CConnectionHandle& connHandler 
/// @return std::nullopt on success.
/// std::unknown on all other failures
std::optional<ble::CConnectionHandle::Error> discover_client_services(ble::CConnectionHandle& connHandler)
{
    int32_t result = ble_gattc_disc_all_svcs(connHandler.handle(), service_discovery_event_handler, &connHandler);
    if (result == static_cast<int32_t>(ble::NimbleErrorCode::success))
        return std::nullopt;

    return std::optional<ble::CConnectionHandle::Error> { ble::CConnectionHandle::Error {
        .code = ble::NimbleErrorCode::unexpectedFailure,
        .msg = "Unknown error received. Return code from nimble: " + std::to_string(result)
    }};
}
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
CConnectionHandle::CConnectionHandle()
    : m_id { INVALID_HANDLE_ID }
{}
CConnectionHandle::~CConnectionHandle()
{
    std::printf("CConnectionHandle destructor\n");
    // Q: how to handle errors in destructors? thinking crash the program is the best since if a destructor fails, something very serious has happened.
    //int result = drop(BLE_HS_ENOENT);
//
    //if (result != SUCCESS && result != BLE_HS_ENOTCONN)
    //    LOG_ERROR_FMT("ERROR terminating connection! ERROR={}", nimble_error_to_string(result));
    reset();
}
CConnectionHandle::CConnectionHandle(CConnectionHandle&& other) noexcept 
    : m_id {other.m_id}
    , m_services {std::move(other.m_services)}
{}
CConnectionHandle& CConnectionHandle::operator=(CConnectionHandle&& other)
{
    /*
        1. Clean up all visible resources
        2. Transfer the content of other into this
        3. Leave other in a valid but undefined state
    */
    // Check if other exists?
    m_id = other.m_id;
    m_services = std::move(other.m_services);
    return *this;
}
void CConnectionHandle::set_connection(uint16_t id)
{ 
    m_id = id; 
    if (!m_services.empty())
    {
        m_services.clear();
    }
}
uint16_t CConnectionHandle::handle() const
{ return m_id; }

/// @brief 
/// @param int32_t reason 
/// @return std::nullopt on success. 
/// NimbleErrorCode::noConnection if there is no connection with the specified handle. 
/// NimbleErrorCode::unknown on failure.
std::optional<CConnectionHandle::Error> CConnectionHandle::drop(NimbleErrorCode reason)
{
    int32_t result = ble_gap_terminate(m_id, static_cast<int32_t>(reason));
    if(result == static_cast<int32_t>(NimbleErrorCode::success))
    {
        reset();
        ASSERT(m_id == INVALID_HANDLE_ID, "Tried to reset a valid connection!");
		return std::nullopt; 
    }

	if(result == static_cast<int32_t>(NimbleErrorCode::noConnection))
	{
		return std::optional<Error>{ Error {
			.code = NimbleErrorCode::noConnection,
			.msg = "no existing connection"			
		}};
	}
	else
	{
		return std::optional<Error>{ Error {
			.code = NimbleErrorCode::unexpectedFailure,
			//.msg = std::format("Unknown error recieved.. Return code from nimble: \"{}\"", result);
            .msg = "Unknown error received. Return code from nimble: " + std::to_string(result)
		}};
	}
}
void CConnectionHandle::reset()
{
    m_id = INVALID_HANDLE_ID;
    m_services.clear();
}
void CConnectionHandle::add_service(const BleClientService& service)
{
    ASSERT(service.handleStart != 0, "Tried to add a service to an invalid/wrong connection");
    m_services.emplace_back(service);
}
int32_t CConnectionHandle::num_services() const
{ return m_services.size(); }
std::vector<BleClientService> CConnectionHandle::services() const
{ return m_services; }
///////////////////
///		CGap	///
///////////////////
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
std::optional<CConnectionHandle::Error> CGap::discover_services()
{
    ASSERT(m_CurrentConnectionHandle.handle() != INVALID_HANDLE_ID, "Tried to intiate 'Service Discovery' on an invalid connection");
    return discover_client_services(m_CurrentConnectionHandle);	// inverted
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
