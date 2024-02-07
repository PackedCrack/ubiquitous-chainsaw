#include "CGapService.h"


namespace nimble
{

    namespace 
    {


    } // namespace

    CAdvertiseFields::CAdvertiseFields(const char* deviceName) 
    : m_fields{}
    {
        assert(sizeof(m_fields) == 96u);
        std::memset(&m_fields, 0, sizeof(m_fields)); // bitfields fucks up static_assert()
        assert(sizeof(m_fields) == 96u);

        // set device name
        const unsigned int IS_COMPLETE = 1;
        m_fields.name = (uint8_t*)deviceName; 
        m_fields.name_len = strlen(deviceName);
        m_fields.name_is_complete = IS_COMPLETE; // meaning there is no need to send additional adv data packets
        assert(m_fields.name != 0);
        assert(m_fields.name_len != 0);
        assert(m_fields.name_is_complete == IS_COMPLETE);

        // set flags
        m_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
        assert( (m_fields.flags & BLE_HS_ADV_F_DISC_GEN) );
        assert( (m_fields.flags & BLE_HS_ADV_F_BREDR_UNSUP) );

        // tx_pwr is the transmit power level of the devices radio signal  (not required to set btw)
        const unsigned int IS_POWER_PRESENT = 1u;
        m_fields.tx_pwr_lvl_is_present = IS_POWER_PRESENT;
        m_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO; // set the power level automatically
        assert(m_fields.tx_pwr_lvl_is_present == IS_POWER_PRESENT);
        assert(m_fields.tx_pwr_lvl != 0);


        // no need to advertise services. since the client will learn them when connecting

        //const uint8_t NUM_SERVICES = 1u; // requires ble_uuid_t which is an uint8_t;
        //const uint16_t SERVICE = 0;
        //ble_uuid16_t servicesArray[NUM_SERVICES];
        //servicesArray[SERVICE] = BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID);
        //m_fields.uuids16 = servicesArray;
        //assert(m_fields.uuids16 == servicesArray);
        //
        //m_fields.num_uuids16 = NUM_SERVICES;
        //assert(m_fields.num_uuids16 = NUM_SERVICES);
        //m_fields.uuids16_is_complete = 1u; // no need to send additional data packets containing services
    }

    const ble_hs_adv_fields& CAdvertiseFields::data() const
    { return m_fields; }


    bool CAdvertiseFields::is_flagged(const uint8_t flag) const
    { return (m_fields.flags & flag); }


    int CAdvertiseFields::configure_fields() const
    {   return ble_gap_adv_set_fields(&m_fields); }


    CAdvertiseParams::CAdvertiseParams() 
    : m_params {}
    {
        static_assert(std::is_trivially_copyable<ble_gap_adv_params>::value);
        std::memset(&m_params, 0, sizeof(m_params));
        
        m_params.conn_mode = BLE_GAP_CONN_MODE_UND;
        m_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    }

    const ble_gap_adv_params& CAdvertiseParams::data() const
    { return m_params; }


    CGapService::CGapService(const char* deviceName, const uint8_t addrType) 
        : m_bleAddressType {addrType}
        , m_fields {deviceName}
        , m_params {}
    {
    }


    void CGapService::advertise() 
    {
        int result;
        result = m_fields.configure_fields();
        if (result != 0) 
            LOG_FATAL_FMT("Error setting advertisement data! result = %d", result);

        result = ble_gap_adv_start(m_bleAddressType, NULL, BLE_HS_FOREVER, &m_params.data(), gap_cb_handler, NULL);
        if (result != 0) {
            LOG_FATAL_FMT("Error starting advertisement = %d", result);
            return;
        }
    }

    int CGapService::gap_cb_handler(struct ble_gap_event *event, void *arg) 
    {
                // docs reference for gap events
        // https://mynewt.apache.org/latest/tutorials/ble/bleprph/bleprph-sections/bleprph-gap-event.html?highlight=ble_gap_event_connect

        // No restrictions on NimBLE operations
        // All context data is transient
        LOG_INFO("Server gap callback was triggered");

        //struct ble_gap_conn_desc desc;
        //int result;


        switch (event->type) {
            case BLE_GAP_EVENT_CONNECT:
                LOG_INFO("BLE_GAP_EVENT_CONNECT");
                break;
            case BLE_GAP_EVENT_DISCONNECT:
                LOG_INFO("BLE_GAP_EVENT_DISCONNECT");
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
           //    LOG_INFO("BLE_GAP_EVENT_DISC_COMPLETE");
           //    break;
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

    
} // namespace nimble