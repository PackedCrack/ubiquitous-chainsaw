
#include "CGapService.hpp"


namespace application
{

namespace 
{

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


auto gap_event_handler = [](ble_gap_event* event, void* arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
        {
            LOG_INFO("BLE_GAP_EVENT_CONNECT");
            // it stops advertising automatically here
            //advertiser.end_advertise();

            // This is how to terminate a connection we dont want by reason of authentication failure
            //https://github.com/espressif/esp-idf/issues/8555
            //uint16_t connectionhandle = event->connect.conn_handle;
            //int result = ble_gap_terminate(connectionhandle, 5);
            //if (result != 0)
            //    LOG_INFO_FMT("UNABLE TO TERMINATE CONNECTION. CODE: {}", result);

            // when connection happens, it is possible to configure another callback that should be used for that connection
            break;
        }
        case BLE_GAP_EVENT_DISCONNECT: 
        {
            LOG_INFO("BLE_GAP_EVENT_DISCONNECT");
            // it does no start advertising automatically

            // Instead of start advertisem,ent here we make a call to try and initiate a connection to a bonded device
            // or do we want the client to be then one always wanting to connect?. Yes.

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


}// namespace


CGapService::CGapService() 
    : m_bleAddressType {255u} // TODO: How to use a pre defined variable? include a common header?
    , m_params { make_advertise_params() }
{
}



void CGapService::initilize(const std::string_view deviceName, uint8_t addressType)
{
     ble_svc_gap_init();
     
    // which one to use? Shoudl we have a bool isInitilized? for saftey
    m_bleAddressType = addressType;
    set_adv_fields(deviceName);

    int result = ble_gap_adv_start(m_bleAddressType, NULL, BLE_HS_FOREVER, &m_params, gap_event_handler, NULL);
    if (result != 0)
        LOG_INFO_FMT("Tried to start advertising. Reason: {}, 2=already started, 6=max num connections already", result);
}

    
} // namespace nimble