
#include "CGapService.hpp"


namespace application
{

namespace 
{

// https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Assigned_Numbers/out/en/Assigned_Numbers.pdf?v=1707124555335
//#define GATT_SVR_SVC_ATTRI_UUID 0x1801
//#define GATT_SVR_SVC_ALERT_UUID 0x1811

// TODO OUTVARIABLES
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


[[Nodiscard]] ble_hs_adv_fields make_advertise_fields(const std::string_view deviceName) // Nodiscard directive ignored for deviecName??
{
    // from what i can tell by testing a similar scenario in visual studio. The constructor of ble_hs_adv_fields will only be called once, and nothing more
    ble_hs_adv_fields fields {};
    make_field_name(fields, deviceName);
    make_field_flags(fields);
    make_field_transmit_power(fields);
    return fields;
}


[[Nodiscard]] ble_gap_adv_params make_advertise_params() 
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



}// namespace


CAdvertiser::CAdvertiser() 
    : m_advParams {make_advertise_params()}
    //,  m_gap_advertise_event_handler {test_func}
{
}

void CAdvertiser::begin_advertise() 
{
    std::printf("Begin advertise!\n");
}

void CAdvertiser::end_advertise() 
{
    std::printf("End advertise!\n");
}

void CGapService::configure(const std::string_view deviceName, uint8_t addressType)
{
    // which one to use? Shoudl we have a bool isInitilized? for saftey
    assert(m_isAdvertising == false);

    if (m_isAdvertising)
        return;

    m_bleAddressType = addressType;
    set_adv_fields(deviceName);

    int result = ble_gap_adv_start(m_bleAddressType, NULL, BLE_HS_FOREVER, &m_params, gap_event_handler, NULL);
    if (result != 0)
        LOG_INFO_FMT("Tried to start advertising. Reason: {}, 2=already started, 6=max num connections already", result);
}


CGapService::CGapService() 
    : m_bleAddressType {255u} // TODO: change this to an application:: lobal defined error code
    , m_params { make_advertise_params() }
    , m_isAdvertising {false}
{
    std::printf("Custom constructor of GapService called!\n");
    // move these out
    assert(m_params.conn_mode & BLE_GAP_CONN_MODE_UND);
    assert(m_params.disc_mode & BLE_GAP_DISC_MODE_GEN);
}



int CGapService::gap_event_handler(ble_gap_event* event, void* arg) 
{
     // docs reference for gap events
    // https://mynewt.apache.org/latest/tutorials/ble/bleprph/bleprph-sections/bleprph-gap-event.html?highlight=ble_gap_event_connect
    // No restrictions on NimBLE operations
    // All context data is transient
    // If advertising results in a connection, the connection inherits this callback as its event-reporting mechanism.

    //struct ble_gap_conn_desc desc;
    //int result;
   
    // static advertise {}
    //static_assert(std::is_trivially_copyable_v<ble_gap_adv_params>::value);
    //static CAdvertiser advertiser {m_params}; // as lvalue or rvalue? its a trivially copiable struct 

/*    CHATGPT
No, you do not need to advertise to find a bonded device in a Bluetooth Low Energy (BLE) context.
Bonding is a process where two devices establish a trusted relationship by exchanging encryption keys 
and storing them for future use. Once two devices have bonded, they can communicate securely without 
needing to perform certain security procedures again. This is useful for scenarios where you want to 
maintain a secure connection between devices over multiple sessions.
When you want to establish a connection with a bonded device, you typically use the device's address or 
identity information stored during the bonding process. Advertising is not necessary because the bonded 
device's address or identity information can be used to directly initiate a connection attempt.
In summary, bonding establishes a trusted relationship between devices, and once bonded, you can use the
 stored information to connect without needing to advertise. However, advertising is still relevant for 
 discovering and connecting to new devices that are not yet bonded.
*/

/*
TESTING:

ble_gap_adv_start(m_bleAddressType, NULL, BLE_HS_FOREVER, &m_params, gap_event_handler, NULL);

This will start advertising. When a client connects, it does not end advertising.
The one that triggered the callback, will inherit the callback aswell.
A new advertise procedure will be initiated (see below): (a different thread i assume)
I (1068) NimBLE: GAP procedure initiated: advertise; 
I (1068) NimBLE: disc_mode=2
I (1068) NimBLE:  adv_channel_map=0 own_addr_type=1 adv_filter_policy=0 adv_itvl_min=0 adv_itvl_max=0
I (1068) NimBLE:

TESTING:
When the client disconnects, the procedure gets removed. (i hope), and advertising on the other threads will continue

TESTING:
If when a client disconnects and i call ble_gap_adv_stop(); the following is shown:
I (42788) NimBLE: GAP procedure initiated: stop advertising.
The advertising will continue anyway.

TESTING:
if when a client connects and i call ble_gap_adv_stop();
The advertising will still continue

TESTING:
Calling start advertise, if already advertising, the following error code is returned:
BLE_HS_EALREADY 0x02 Operation already in progress or completed.

https://github.com/espressif/esp-idf/issues/4243
t he Controller shall continue advertising until the Host issues an HCI_LE_Set_Advertising_Enable command with 
Advertising_Enable set to 0x00 (Advertising is disabled) or until a connection is created or until the Advertising 
is timed out due to high duty cycle Directed Advertising. 

TESTING:
if i set the max number of connections to 1 in sdk configs
starting advertising after a connection will result in error code:
BLE_HS_ENOMEM 0x06 Operation failed due to resource exhaustion.

*/

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
}



    
} // namespace nimble