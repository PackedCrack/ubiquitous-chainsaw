#include "CGapService.hpp"


namespace nimble
{

namespace 
{

// https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Assigned_Numbers/out/en/Assigned_Numbers.pdf?v=1707124555335
#define GATT_SVR_SVC_ATTRI_UUID 0x1801
#define GATT_SVR_SVC_ALERT_UUID 0x1811


void make_field_name(ble_hs_adv_fields& fields, const std::string_view deviceName)
{
    const unsigned int IS_COMPLETE = 1u;
    fields.name = (uint8_t*)deviceName.data();
    fields.name_len = static_cast<uint8_t>(deviceName.size());
    fields.name_is_complete = IS_COMPLETE;
    assert(fields.name != 0);
    assert(fields.name_len == deviceName.size());
    assert(fields.name_is_complete == IS_COMPLETE);
}


 void make_field_flags(ble_hs_adv_fields& fields)
 {

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    assert((fields.flags & BLE_HS_ADV_F_DISC_GEN));
    assert((fields.flags & BLE_HS_ADV_F_BREDR_UNSUP));
 }


void make_field_transmit_power(ble_hs_adv_fields& fields)
{
       // tx_pwr is the transmit power level of the devices radio signal  (not required to set btw)
       const unsigned int IS_POWER_PRESENT = 1u;
       fields.tx_pwr_lvl_is_present = IS_POWER_PRESENT;
       fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO; // set the power level automatically
       assert(fields.tx_pwr_lvl_is_present == IS_POWER_PRESENT);
       assert(fields.tx_pwr_lvl != 0);
}


[[NoDiscard]] ble_hs_adv_fields make_advertise_fields(const std::string_view deviceName) // NoDiscard directive ignored for deviecName??
{
    ble_hs_adv_fields fields {};
    make_field_name(fields, deviceName);
    make_field_flags(fields);
    make_field_transmit_power(fields);

    return fields;
}


[[NoDiscard]] ble_gap_adv_params make_advertise_params() 
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


}// namespace


CGapService::CGapService(const std::string_view deviceName, const uint8_t addrType) 
    : m_bleAddressType {addrType}
    , m_params { make_advertise_params() }
{
    
    assert(m_params.conn_mode & BLE_GAP_CONN_MODE_UND);
    assert(m_params.disc_mode & BLE_GAP_DISC_MODE_GEN);

    ble_hs_adv_fields fields = make_advertise_fields(deviceName);
    int result;
    result = ble_gap_adv_set_fields(&fields);
    if (result != 0) 
        LOG_FATAL_FMT("Error setting advertisement data!: %d", result);



    result = ble_gap_adv_start(m_bleAddressType, NULL, BLE_HS_FOREVER, &m_params, gap_event_handler, NULL);
    if (result != 0)
        LOG_FATAL_FMT("Error starting advertisement: %d", result);
    
}


int CGapService::gap_event_handler(ble_gap_event* event, void* arg) 
{
     // docs reference for gap events
    // https://mynewt.apache.org/latest/tutorials/ble/bleprph/bleprph-sections/bleprph-gap-event.html?highlight=ble_gap_event_connect
    // No restrictions on NimBLE operations
    // All context data is transient
    // If advertising results in a connection, the connection inherits this callback as its event-reporting mechanism.
    //LOG_INFO("Server gap callback was triggered");
    //struct ble_gap_conn_desc desc;
    //int result;
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            LOG_INFO("BLE_GAP_EVENT_CONNECT");
            // stop advertising
            int result;
            //ble_gap_adv_stop();
            if (result != 0)
                LOG_FATAL_FMT("Error Stopping advertising: %d", result);
            
            LOG_INFO("Advertising has stopped!");
        
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            LOG_INFO("BLE_GAP_EVENT_DISCONNECT");
            // start advertising again
            //advertise(); how to advertise again?..
            // should the params be stored as a global variable in this translation unit? OOP lovers are crying by that thought..
            LOG_INFO("Advertising has started!");
            break;
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
}



    
} // namespace nimble