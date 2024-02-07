#include "CGattServer.h"




namespace nimble
{


    namespace
    {
    
    }

    CGattServer::CGattServer(const char* deviceName, const uint8_t addrType) 
        :m_gap{deviceName, addrType} 
    {
        m_gap.advertise(gap_cb_handler);
    }


    int CGattServer::gap_cb_handler(struct ble_gap_event* event, void* arg) 
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
                m_gap.advertise(gap_cb_handler);
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
            //case BLE_GAP_EVENT_IDENTITY_RESOLVED:
            //    LOG_INFO("BLE_GAP_EVENT_IDENTITY_RESOLVED");
            //    break;
            //case BLE_GAP_EVENT_REPEAT_PAIRING:
            //    LOG_INFO("BLE_GAP_EVENT_REPEAT_PAIRING");
            //    break;
            //case BLE_GAP_EVENT_PHY_UPDATE_COMPLETE:
            //    LOG_INFO("BLE_GAP_EVENT_PHY_UPDATE_COMPLETE");
            //    break;
            //case BLE_GAP_EVENT_EXT_DISC:
            //    LOG_INFO("BLE_GAP_EVENT_EXT_DISC");
            //    break;
            //case BLE_GAP_EVENT_PERIODIC_SYNC:
            //    LOG_INFO("BLE_GAP_EVENT_PERIODIC_SYNC");
            //    break;
            //case BLE_GAP_EVENT_PERIODIC_REPORT:
            //    LOG_INFO("BLE_GAP_EVENT_PERIODIC_REPORT");
            //    break;
            //case BLE_GAP_EVENT_PERIODIC_SYNC_LOST:
            //    LOG_INFO("BLE_GAP_EVENT_PERIODIC_SYNC_LOST");
            //    break;
            //case BLE_GAP_EVENT_SCAN_REQ_RCVD:
            //    LOG_INFO("BLE_GAP_EVENT_SCAN_REQ_RCVD");
            //    break;
            //case BLE_GAP_EVENT_PERIODIC_TRANSFER:
            //    LOG_INFO("BLE_GAP_EVENT_PERIODIC_TRANSFER");
            //    break;
            //case BLE_GAP_EVENT_PATHLOSS_THRESHOLD:
            //    LOG_INFO("BLE_GAP_EVENT_PATHLOSS_THRESHOLD");
            //    break;
            //case BLE_GAP_EVENT_TRANSMIT_POWER:
            //    LOG_INFO("BLE_GAP_EVENT_TRANSMIT_POWER");
            //    break;
            //case BLE_GAP_EVENT_SUBRATE_CHANGE:
            //    LOG_INFO("BLE_GAP_EVENT_SUBRATE_CHANGE");
            //    break;
            //case BLE_GAP_EVENT_VS_HCI:
            //    LOG_INFO("BLE_GAP_EVENT_VS_HCI");
            //    break;
            //case BLE_GAP_EVENT_REATTEMPT_COUNT:
            //    LOG_INFO("BLE_GAP_EVENT_REATTEMPT_COUNT");
            //    break;
            default: 
                break;
        } // switch
        return 0;
    }


} // namesapce nimble