#include "CAdvertiseFields.h"


namespace nimble 
{

    #define GATT_SVR_SVC_ALERT_UUID 0x1811
    #define GATT_SVR_SVC_ATTRI_UUID 0x1801

    namespace 
    {
        void print_adv_field_flags(const ble_hs_adv_fields& field) 
        {
            if (field.flags & BLE_HS_ADV_F_DISC_GEN) 
                LOG_INFO("Device Discover Mode: General");

            if (field.flags & BLE_HS_ADV_F_DISC_LTD)
                LOG_INFO("Device Discover Mode: Limited");

            if (field.flags & BLE_HS_ADV_F_BREDR_UNSUP)
                LOG_INFO("Device Bluetooth Classic Support: NONE");
        }

        void print_adv_field_signal_power(const ble_hs_adv_fields& field)
        {
            // Lower then number, the weaker the signal
            static const unsigned int IS_PRESENT = 1u;
            if (field.tx_pwr_lvl_is_present == IS_PRESENT) 
            {
                if (field.tx_pwr_lvl == BLE_HS_ADV_TX_PWR_LVL_AUTO)  
                    LOG_INFO("Device Signal Strength Setting: AUTO");
                else 
                {
                    const int8_t PWR_LVL = field.tx_pwr_lvl;
                    LOG_INFO_FMT("Device Signal Strength Setting: %d", PWR_LVL);
                    //ESP_LOGI(SERVER_TAG, "Device Signal Strength Setting: %d", PWR_LVL);
                }
            }
            else 
                LOG_INFO("Device Signal Strength Setting: NOT SET");
        }

    } // namespace


    CAdvertiseFields::CAdvertiseFields(const char* deviceName) 
    : m_fields{}
    {
        // initilize
        assert(sizeof(m_fields) == 96u);
        std::memset(&m_fields, 0, sizeof(m_fields));
        assert(sizeof(m_fields) == 96u);

        // set name
        const unsigned int IS_COMPLETE = 1;
        m_fields.name = (uint8_t*)deviceName; 
        m_fields.name_len = strlen(deviceName);
        m_fields.name_is_complete = IS_COMPLETE; // meaning there is no need to send additional adv data packets
        assert(m_fields.name != 0);
        assert(m_fields.name_len != 0);
        assert(m_fields.name_is_complete == IS_COMPLETE);

        // set flags
        m_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
        assert(m_fields.flags != 0);
        print_adv_field_flags(m_fields);

        // tx_pwr is the transmit power level of the devices radio signal  (not required to set btw)
        const unsigned int IS_POWER_PRESENT = 1u;
        m_fields.tx_pwr_lvl_is_present = IS_POWER_PRESENT;
        m_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO; // set the power level automatically
        assert(m_fields.tx_pwr_lvl_is_present == IS_POWER_PRESENT);
        assert(m_fields.tx_pwr_lvl != 0);
        print_adv_field_signal_power(m_fields);

        // TODO: GENERALIZE THIS
        const uint8_t NUM_SERVICES = 1; // requires ble_uuid_t which is an uint8_t;
        const uint16_t SERVICE_1 = 0;
        //const uint16_t SERVICE_2 = 1;

        ble_uuid16_t servicesArray[NUM_SERVICES];
        servicesArray[SERVICE_1] = BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID);
        //servicesArray[SERVICE_2] = BLE_UUID16_INIT(GATT_SVR_SVC_ATTRI_UUID);
        m_fields.uuids16 = servicesArray;

        m_fields.num_uuids16 = NUM_SERVICES;
        m_fields.uuids16_is_complete = 1u; // no need to send additional data packets containing services
    }

    ble_hs_adv_fields& CAdvertiseFields::data()
    {   
        return m_fields;
    }

    bool CAdvertiseFields::is_flagged(const uint8_t flag) const
    { return (m_fields.flags & flag); }

} // namespace nimble
