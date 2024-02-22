
#include "CGap.hpp"


namespace ble
{


namespace 
{

//constexpr std::string_view deviceName = "Chainsaw-server";
constexpr uint16_t INVALID_HANDLE = 65535u;
constexpr uint8_t INVALID_ADDRESS_TYPE = 255u;
uint16_t currentConnectionHandle = INVALID_HANDLE;

//TimerHandle_t timer_handle;
//
//void timer_callback(TimerHandle_t xTimer) {
//    printf("Timer expired!\n");
//
//    // drop current connection
//    // stop timer
//}
//
//void start_timer() {
//    timer_handle = xTimerCreate("MyTimer", pdMS_TO_TICKS(5000), pdFALSE, NULL, timer_callback);
//    xTimerStart(timer_handle, 0);
//}
//
//void reset_timer() {
//    xTimerReset(timer_handle, 0);
//}
//
//void stop_timer() {
//    xTimerStop(timer_handle, 0);
//    xTimerDelete(timer_handle, 0);
//}



struct BleCharacteristic
{
    ble_uuid_any_t uuid;
    uint16_t handle;
    uint16_t handleValue;
    uint8_t properties;
};

struct BleService 
{
    ble_uuid_any_t uuid;
    uint16_t connHandle;
    uint16_t handleStart;
    uint16_t handleEnd;
    std::vector<BleCharacteristic> characteristics;
};

std::vector<BleService> foundServices {};

// https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Assigned_Numbers/out/en/Assigned_Numbers.pdf?v=1707124555335
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


auto service_discovery_event_handler = [](uint16_t conn_handle,
                    const struct ble_gatt_error *error,
                    const struct ble_gatt_svc *service,
                    void *arg) {

    int result = error->status;
    
    if (result == BLE_HS_EDONE)
    {
        LOG_INFO("Discovery Process has ended!");
        LOG_INFO_FMT("Number of services: {}", foundServices.size());
        return 0;
    }

    if (result != ESP_OK)
    {
        LOG_WARN_FMT("Discovery Process error: {}", result);
        return 1; // TODOD CHECK WHAT TO RETURN
    }

    foundServices.emplace_back(
        BleService {
            .uuid = service->uuid,
            .connHandle = conn_handle,
            .handleStart = service->start_handle,
            .handleEnd = service->end_handle,
            .characteristics {}
        }
    );

    discover_service_characteristics(conn_handle, service->start_handle, service->end_handle);


    //result = ble_gattc_disc_all_dscs(conn_handle, service->start_handle, service->end_handle, descriptor_discovery_event_handler, NULL);
    //if (result != 0) {
    //    LOG_INFO_FMT("Failed to initiate descriptor discovery. Error: {}", result);
    //}    
    // 6=BLE_GATT_OP_DISC_ALL_DSCS // so failed to discovery any since there were none there
    // also 6, BLE_HS_ENOMEM

    // 14=BLE_HS_EDONE
return 0;
};



void discover_client_services(uint16_t connHandler)
{
    int result = ble_gattc_disc_all_svcs(connHandler, service_discovery_event_handler, NULL);
    if (result != 0) 
    {
        LOG_INFO_FMT("Failed to start Service Discovery process: {}", result);
    }
}


auto gap_event_handler = [](ble_gap_event* event, void* arg) {

    //static CAdvertiser advertiser{};
    CGap* gap = static_cast<CGap*>(arg);// shared ownership?
    

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
        {
            LOG_INFO("BLE_GAP_EVENT_CONNECT");
            gap->end_advertise();
            // it stops advertising automatically here
            //advertiser.end_advertise();

            // This is how to terminate a connection we dont want by reason of authentication failure
            //https://github.com/espressif/esp-idf/issues/8555
            //uint16_t connectionhandle = event->connect.conn_handle;
            //int result = ble_gap_terminate(connectionhandle, 5);
            //if (result != 0)
            //    LOG_INFO_FMT("UNABLE TO TERMINATE CONNECTION. CODE: {}", result);

            // when connection happens, it is possible to configure another callback that should be used for that connection

            // unable to have locks in here if new procedures are to be created

            //discover_client_services(event->connect.conn_handle);

            if(currentConnectionHandle != INVALID_HANDLE)
                break;
            
            currentConnectionHandle = event->connect.conn_handle;
            LOG_INFO_FMT("Current handle: {}", currentConnectionHandle);
            break;
        }
        case BLE_GAP_EVENT_DISCONNECT: 
        {
            LOG_INFO("BLE_GAP_EVENT_DISCONNECT");
            gap->begin_advertise();
            //uint16_t connectionhandle = event->connect.conn_handle;
            //int result = ble_gap_terminate(connectionhandle, 5);
            //if (result != 0)
            //    LOG_INFO_FMT("UNABLE TO TERMINATE CONNECTION. CODE: {}", result); // error code 7.

            //foundServices.clear();
            currentConnectionHandle = INVALID_HANDLE;
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



CGap::CGap() 
    : m_bleAddressType {INVALID_ADDRESS_TYPE} // How to make this better? cant determine bleaddresstype until nimble host stack is started
    , m_params { make_advertise_params() }
    , m_isAdvertising {false}
    
{
}


//~CGap::CGap()
//{
//    if(m_isAdvertising)
//        end_advertise();
//}


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
    if(currentConnectionHandle == INVALID_HANDLE)
        return;

    int8_t rssiValue {};
    int rssi = ble_gap_conn_rssi(currentConnectionHandle, &rssiValue);
    if (rssi != 0)
    {
        LOG_WARN_FMT("Unable to retrieve rssi value: {}", rssi);
    }
    else
    {
        LOG_INFO_FMT("RSSI VALUE: {}", rssiValue);
    }
}


void CGap::begin_advertise()
{
    if (m_isAdvertising)
    {
        LOG_WARN("GAP service is already advertising!");
        return;
    }

    int result = ble_gap_adv_start(m_bleAddressType, NULL, BLE_HS_FOREVER, &m_params, gap_event_handler, this); // shared ownership?
    if (result != 0)
    {
        LOG_WARN_FMT("Tried to start advertising. Reason: {}, 2=already started, 6=max num connections already", result);
        return;
    }

    m_isAdvertising = true;
}


void CGap::end_advertise()
{
    if (!m_isAdvertising)
    {
        LOG_WARN("GAP service isn't advertising!");
        return;
    }

    ble_gap_adv_stop();
    m_isAdvertising = false;
    
}

} // namespace application

