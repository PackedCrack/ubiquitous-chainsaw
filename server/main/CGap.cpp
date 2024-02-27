
#include "CGap.hpp"



namespace ble
{

std::promise<void> syncPromise {};
std::future<void> syncFuture = syncPromise.get_future();

bool callback_finished = false;


namespace 
{

std::vector<BleService> foundServices {};

uint8_t* make_field_name(const std::string_view deviceName)
{ return (uint8_t*)deviceName.data(); }

uint8_t make_field_name_len(const std::string_view deviceName)
{ return static_cast<uint8_t>(deviceName.size()); }

unsigned int make_field_name_is_complete()
{ return 1u; }

uint8_t make_field_flags()
{ return BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP; }

unsigned int make_field_tx_pwr_is_present()
{ return 1u; }

int8_t make_field_pwr_lvl()
{ return BLE_HS_ADV_TX_PWR_LVL_AUTO; }

ble_hs_adv_fields make_advertise_fields(const std::string_view deviceName) // Nodiscard directive ignored for deviecName??
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


void set_adv_fields(const std::string_view deviceName)
{
    ble_hs_adv_fields fields = make_advertise_fields(deviceName); // only the constructor for ble_hs_adv_fields will be called here
    int result;
    result = ble_gap_adv_set_fields(&fields);
    if (result != 0) 
        LOG_FATAL_FMT("Error setting advertisement data!: %d", result);
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
//    std::string_view uuid = ble_uuid_to_str((const ble_uuid_t* )&dsc->uuid, uuidBuf);
//    LOG_INFO_FMT("Descriptor discovered: handle={}, UUID={}", dsc->handle, uuid);
//    return 0;
//};


void print_services()
{
    if (foundServices.empty())
        return;

    

    for (auto& service : foundServices)
    {
        const int MAX_UUID_LEN = 128;
        char serviceUuidBuf [MAX_UUID_LEN];
        std::string_view tmpServiceUuid = ble_uuid_to_str((const ble_uuid_t* )&service.uuid, serviceUuidBuf);

        LOG_INFO_FMT("Service: UUID={} connHandle={} handleStart={} handleEnd={}", tmpServiceUuid, service.connHandle, 
                                                                                    service.handleStart, service.handleEnd);

        for (auto& characteristic : service.characteristics)
        {
            char serviceUuidBuf [MAX_UUID_LEN];
            std::string_view tmpCharacterUuid = ble_uuid_to_str((const ble_uuid_t* )&characteristic.uuid, serviceUuidBuf);
            LOG_INFO_FMT("Characteristic: UUID={} handle={} handleValue={} properties={}", tmpCharacterUuid, characteristic.handle, 
                                                                                            characteristic.handleValue, characteristic.properties);
        }
    }
}

// 3=BLE_HS_EINVAL  invalid argument
// 10=BLE_HS_EBADDATA   Command from peer is invalid. bad data format etc
// or 10= BLE_ATT_ERR_ATTR_NOT_FOUND  No attribute found within the given attribute handle range.
auto characteristic_discovery_event_handler = [](uint16_t conn_handle,
                                            const struct ble_gatt_error *error,
                                            const struct ble_gatt_chr *chr,
                                            void *arg) {
    // https://mynewt.apache.org/master/network/ble_hs/ble_hs_return_codes.html#return-codes-att                                            
    int result = error->status;
    if (result == BLE_HS_EDONE || result == BLE_HS_EINVAL)
    {
        // dont know why it wont return BLE_HS_EDONE, but the last thing it does return is BLE_HS_EINVAL for some reason
        LOG_INFO_FMT("Finished with characteristic discovery: {}", result);
        print_services();
        return 0;
    }

    if (result != ESP_OK) 
    {
        LOG_WARN_FMT("Error during characteristic discovery: {}", result);
        return 1;
    } 

    // Find the service to which this characteristic belongs to
    for (auto& service : foundServices)
    {
        if (chr->def_handle >= service.handleStart && chr->def_handle <= service.handleEnd)
        {
            service.characteristics.emplace_back(
                BleCharacteristic {
                    .uuid = chr->uuid,
                    .handle = chr->def_handle,
                    .handleValue = chr->val_handle,
                    .properties = chr->properties
                }
            );
            break;
        }
    }

    return 0;
};


void discover_service_characteristics(uint16_t connhandle, uint16_t handleStart, uint16_t handleEnd)
{
    if (foundServices.empty())
        return;

    int result = ble_gattc_disc_all_chrs(connhandle, handleStart, handleEnd, characteristic_discovery_event_handler, NULL);
    if (result != 0) 
    {
        LOG_WARN_FMT("Failed to initiate characteristic discovery. Error: {}", result);
    // return 6,  BLE_HS_ENOMEM Operation failed due to resource exhaustion.

    }
}


auto service_discovery_event_handler = [](uint16_t connHandle,
                    const struct ble_gatt_error *error,
                    const struct ble_gatt_svc *service,
                    void *arg) {
    // https://mynewt.apache.org/v1_8_0/network/ble_hs/ble_hs_return_codes.html
    CConnectionHandle* pConnHandle = static_cast<CConnectionHandle*>(arg);
    assert(pConnHandle->handle() == connHandle);

    int returnedResult = error->status;

    std::printf("callback started");

    if (returnedResult == PROCEDURE_HAS_FINISHED)
    {
        LOG_INFO("Discovery procedure has finished!");
        
        callback_finished = true;

        return PROCEDURE_HAS_FINISHED;
    }

    if (returnedResult == SUCCESS)
    {
        std::printf("SUCCESS");
        pConnHandle->add_service(
            BleService {
                    .uuid = service->uuid,
                    .connHandle = connHandle,
                    .handleStart = service->start_handle,
                    .handleEnd = service->end_handle,
                    .characteristics {}
            }
        );

        char serviceUuidBuf [MAX_UUID_LEN];
        std::string_view serviceUuidToString = ble_uuid_to_str(reinterpret_cast<const ble_uuid_t*>( &pConnHandle->services()[ (pConnHandle->num_services() - 1) ].uuid ), serviceUuidBuf);
        LOG_INFO_FMT("Added Service UUID={}", serviceUuidToString);
        return SUCCESS;
    }

    if (returnedResult != BLE_HS_EDONE)
    {
        LOG_WARN_FMT("Discovery Process error: {}", returnedResult);
        return BLE_HS_EUNKNOWN; // Unexpected failure; catch all.
    }


    //discover_service_characteristics(conn_handle, service->start_handle, service->end_handle);

    //result = ble_gattc_disc_all_dscs(conn_handle, service->start_handle, service->end_handle, descriptor_discovery_event_handler, NULL);
    //if (result != 0) {
    //    LOG_INFO_FMT("Failed to initiate descriptor discovery. Error: {}", result);
    //}    
    // 6=BLE_GATT_OP_DISC_ALL_DSCS // so failed to discovery any since there were none there
    // also 6, BLE_HS_ENOMEM

    assert(0);
    return BLE_HS_EUNKNOWN; // should not be triggered
};



int discover_client_services(CConnectionHandle& connHandler)
{
    return ble_gattc_disc_all_svcs(connHandler.handle(), service_discovery_event_handler, &connHandler);
}


auto gap_event_handler = [](ble_gap_event* event, void* arg) {

    CGap* pGap = static_cast<CGap*>(arg);// shared ownership?
    

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
        {
            LOG_INFO("BLE_GAP_EVENT_CONNECT");

            pGap->set_connection(event->connect.conn_handle);

            int result = pGap->discover_services();
            if (result != SUCCESS)
            {
                LOG_ERROR_FMT("Service Discovery has failed! ERROR_CODE={}", result);
            }

            std::printf("i was ehre!\n");
            //uint16_t handle = pGap->connection_handle();
            //assert(handle != INVALID_HANDLE_ID);

            //int result = pGap->drop_connection(BLE_ERR_AUTH_FAIL); // will trigger disconnect callback
            //if (result == BLE_HS_ENOTCONN)
            //    LOG_ERROR_FMT("Tried to terminate a non existent connection. ERROR_CODE={}", result);
            //else if (result != 0)
            //    LOG_ERROR_FMT("Terminating connection failed due to unspecified error. ERROR_CODE={}", result);
            
    
            //int result = discover_client_services(m_current);


            // when connection happens, it is possible to configure another callback that should be used for that connection

            // unable to have locks in here if new procedures are to be created

            //discover_client_services(event->connect.conn_handle);

            //if(currentConnectionHandle != INVALID_HANDLE_ID)
            //    break;
            //
            //currentConnectionHandle = event->connect.conn_handle;
            //LOG_INFO_FMT("Current handle: {}", currentConnectionHandle);
            break;
        }
        case BLE_GAP_EVENT_DISCONNECT: 
        {
            LOG_INFO("BLE_GAP_EVENT_DISCONNECT");
            pGap->reset_connection();
            pGap->begin_advertise();
            //uint16_t connectionhandle = event->connect.conn_handle;
            //int result = ble_gap_terminate(connectionhandle, 5);
            //if (result != 0)
            //    LOG_INFO_FMT("UNABLE TO TERMINATE CONNECTION. CODE: {}", result); // error code 7.

            //foundServices.clear();
            //currentConnectionHandle = INVALID_HANDLE;
            break;
        }

        case BLE_GAP_EVENT_CONN_UPDATE:
            LOG_INFO("BLE_GAP_EVENT_CONN_UPDATE");
            
            break;
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
        default: 
            break;
    } // switch
    return 0;
};


uint8_t ble_generate_random_device_address() 
{
    int result;
    const int RND_ADDR = 1;
    const int PUB_ADDR = 0;
    const uint8_t INVALID_ADDRESS_TYPE = 255u;
    uint8_t addrType = INVALID_ADDRESS_TYPE;

    result = ble_hs_util_ensure_addr(RND_ADDR);
    assert(result == 0);
    result = ble_hs_id_infer_auto(PUB_ADDR, &addrType); // 1/private do not work here, type will depend ble_hs_util_ensure_addr()
    if (result != 0) 
        LOG_FATAL_FMT("No address was able to be inferred %d\n", result);
    
    if (addrType == INVALID_ADDRESS_TYPE)
        LOG_FATAL_FMT("Error address type not determined! %d\n", result);
    
    // print the address
    std::array<uint8_t, 6> bleDeviceAddr {};
    result = ble_hs_id_copy_addr(addrType, bleDeviceAddr.data(), NULL); 
    if (result != 0) 
        LOG_FATAL_FMT("Adress was unable to be assigned %d\n", result);

    std::printf("BLE Device Address: %02x:%02x:%02x:%02x:%02x:%02x \n", bleDeviceAddr[5],bleDeviceAddr[4],bleDeviceAddr[3],bleDeviceAddr[2],bleDeviceAddr[1],bleDeviceAddr[0]);

    return addrType;
}

}// namespace


CConnectionHandle::CConnectionHandle()
    : m_id { INVALID_HANDLE_ID }
{

}


CConnectionHandle::~CConnectionHandle()
{

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


int CConnectionHandle::drop(int reason)
{
    // https://mynewt.apache.org/v1_8_0/network/ble_hs/ble_hs_return_codes.html
    assert(m_id != INVALID_HANDLE_ID);
    //assert(!m_services.empty());

    int result = ble_gap_terminate(m_id, reason);
    if (result != ESP_OK)
    {
        LOG_ERROR_FMT("ERROR terminating connection! ERROR_CODE={}", result);
    }

    return result;
}


void CConnectionHandle::reset_connection()
{
    assert(m_id != INVALID_HANDLE_ID);
    //assert(!m_services.empty());
    m_id = INVALID_HANDLE_ID;
    m_services.clear();
}


void CConnectionHandle::add_service(const BleService& service)
{
    m_services.emplace_back(service);

}

int CConnectionHandle::num_services() const
{ return m_services.size(); }



const std::vector<BleService> CConnectionHandle::services() const
{ return m_services; }


CGap::CGap() 
    : m_bleAddressType {INVALID_ADDRESS_TYPE} // How to make this better? cant determine bleaddresstype until nimble host stack is started
    , m_params { make_advertise_params() }
    //, m_isAdvertising {false}
    , m_currentConnectionHandle {}
{
}


//~CGap::CGap()
//{
//    if(m_isAdvertising)
//        end_advertise();
//}

int CGap::discover_services()
{
    assert(m_currentConnectionHandle.handle() != INVALID_HANDLE_ID);
    int result = discover_client_services(m_currentConnectionHandle);
    return result;
}


void CGap::set_connection(const uint16_t id)
{
    m_currentConnectionHandle.set_connection(id);

}

uint16_t CGap::connection_handle() const
{
    return m_currentConnectionHandle.handle();
}


int CGap::drop_connection(int reason)
{
    // if we drop connection manually, ble will start advertising automatically
    return m_currentConnectionHandle.drop(reason);
}

void CGap::reset_connection()
{
    m_currentConnectionHandle.reset_connection();

}

void CGap::start()
{
    //ble_svc_gap_init(); // will crash if called before nimble_port_init()
    assert(m_bleAddressType == INVALID_ADDRESS_TYPE);
    m_bleAddressType = ble_generate_random_device_address(); // nimble_port_run(); has to be called before this
    assert(m_bleAddressType != INVALID_ADDRESS_TYPE);
    
    std::string_view deviceName = "Chainsaw-server";
    int result;
    result = ble_svc_gap_device_name_set(deviceName.data());
    assert(result == 0);

    set_adv_fields(deviceName);
    begin_advertise();
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


void CGap::begin_advertise()
{
    //assert(!m_isAdvertising);
    //m_isAdvertising = true;
    int result = ble_gap_adv_start(m_bleAddressType, NULL, BLE_HS_FOREVER, &m_params, gap_event_handler, this); // shared ownership?
    if(result != 0)
        LOG_ERROR_FMT("ERORR STARTING ADVERTISE! ERROR_CODE={}", result);

    assert(result == ESP_OK);
}


void CGap::end_advertise()
{
    // NOTE: host will end advertising by itself.. need to test with a second source to make sure
    //assert(m_isAdvertising);
    //m_isAdvertising = false;
    int result = ble_gap_adv_stop();
    if(result != 0)
        LOG_ERROR_FMT("ERORR ENDING ADVERTISE! ERROR_CODE={}", result);

    //assert(result == ESP_OK);
}

} // namespace application


