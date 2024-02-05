#include "CNimble.h"


#include <bitset> // for visual testing


namespace nimble
{

    namespace 
    {


    

        #define SERVER_TAG "Chainsaw-server" // used for ESP_LOG

        void bit_operation_tests() 
        {
            ble_hs_adv_fields fields;
            std::memset(&fields, 0, sizeof(fields));

            std::bitset<8> bitsetValueBefore(fields.flags);
            std::printf("Flag bits before init: \t%s\n", bitsetValueBefore.to_string().c_str());

            fields.flags = BLE_HS_ADV_F_DISC_GEN;
            std::bitset<8> bitsetValueSetDisc(fields.flags);
            std::printf("Set BLE_HS_ADV_F_DISC_GEN: \t%s\n", bitsetValueSetDisc.to_string().c_str());

            fields.flags = fields.flags | BLE_HS_ADV_F_BREDR_UNSUP;
            std::bitset<8> bitsetValueSetSupp(fields.flags);
            std::printf("Set BLE_HS_ADV_F_BREDR_UNSUP: \t%s\n", bitsetValueSetSupp.to_string().c_str());

            std::bitset<8> bitsetFields(fields.flags);
            std::bitset<8> bitsetDisc(BLE_HS_ADV_F_DISC_GEN);
            std::bitset<8> bitsetAnd(fields.flags & BLE_HS_ADV_F_DISC_GEN);
            std::printf("%s AND %s = %s\n", bitsetFields.to_string().c_str(), bitsetDisc.to_string().c_str(), bitsetAnd.to_string().c_str());

        }

        void print_adv_field_flags(const ble_hs_adv_fields& field) 
        {
            /**
            Flag defines meaning: 
                BLE_HS_ADV_FLAGS_LEN     // the lenght of the adv flags field
                BLE_HS_ADV_F_DISC_LTD    // limited discoverable mode (has a specific time duration)
                BLE_HS_ADV_F_DISC_GEN    // General discoverable mode (always discoverable)
                BLE_HS_ADV_F_BREDR_UNSUP  // Devide does not support Bluetooth classic      
            */
  
            if (field.flags & BLE_HS_ADV_F_DISC_GEN) 
                ESP_LOGI(SERVER_TAG, "Device Discover Mode: General");

            if (field.flags & BLE_HS_ADV_F_DISC_LTD)
                ESP_LOGI(SERVER_TAG, "Device Discover Mode: Limited");

            if (field.flags & BLE_HS_ADV_F_BREDR_UNSUP)
                ESP_LOGI(SERVER_TAG, "Device Bluetooth Classic Support: NONE");
        }

        void print_adv_field_signal_power(const ble_hs_adv_fields& field)
        {

            // Lower then number, the weaker the signal
            static const unsigned int IS_PRESENT = 1;

            if (field.tx_pwr_lvl_is_present == IS_PRESENT) 
            {
                if (field.tx_pwr_lvl == BLE_HS_ADV_TX_PWR_LVL_AUTO)  
                    ESP_LOGI(SERVER_TAG, "Device Signal Strength Setting: AUTO");
                else 
                {
                    const int8_t PWR_LVL = field.tx_pwr_lvl;
                    ESP_LOGI(SERVER_TAG, "Device Signal Strength Setting: %d", PWR_LVL);
                }
            }
            else 
               ESP_LOGI(SERVER_TAG, "Device Signal Strength Setting: NOT SET");
        }


        // static variables
        static uint8_t serverAddrType; // will be 1 or 0 depedning on rnd or pub addr

        // Callback function
        static void nimble_on_reset_handle(int reason) 
        {
            LOG_FATAL_FMT("ESP ERROR. Reset due to %d\n", reason);
        }

       static void nimble_on_sync_handle(void) 
       {
            int result;
            const int RND_ADDR = 1;
            const int PUB_ADDR = 0;

            result = ble_hs_util_ensure_addr(RND_ADDR);
            assert(result == 0);

            result = ble_hs_id_infer_auto(PUB_ADDR, &serverAddrType); // 1/private do not work here, type will depend ble_hs_util_ensure_addr() is the addressType needed for otherthings??
            if (result != 0) {
                LOG_FATAL_FMT("No address was able to be infered %d\n", result);
            }
        
            uint8_t bleDeviceAddr[6] {};
            result = ble_hs_id_copy_addr(serverAddrType, bleDeviceAddr, NULL);
            if (result != 0) 
            {
                LOG_FATAL_FMT("Adress was unable to be retrieved %d\n", result);
            }

            // TODO: HOW TO LOG INFO WITH PARAMS?
            if (serverAddrType == RND_ADDR) 
                ESP_LOGI(SERVER_TAG, "BLE Random Address: %02x:%02x:%02x:%02x:%02x:%02x", bleDeviceAddr[5], bleDeviceAddr[4], bleDeviceAddr[3], 
                                                                                            bleDeviceAddr[2], bleDeviceAddr[1], bleDeviceAddr[0]);  // %02x are hexdec placeholders
            else if (serverAddrType == PUB_ADDR) 
                ESP_LOGI(SERVER_TAG, "BLE Public Address: %02x:%02x:%02x:%02x:%02x:%02x", bleDeviceAddr[5], bleDeviceAddr[4], bleDeviceAddr[3], 
                                                                                            bleDeviceAddr[2], bleDeviceAddr[1], bleDeviceAddr[0]);

            gap_advertise();
            
        }

        static void gap_advertise(void)
        {
            ble_gap_adv_params gapAdvConfigParams; // ble adverting configuration parameters
            ble_hs_adv_fields gapAdvFields; // Adv data and scan response (name, services, UUID etc)
            const char *name;
            int result;
        
            //bit_operation_tests();

            // ble_hs_adv_fields has bit fields which fucks up the static_assert. WHAT IS THE BEST WAY TO DO THIS THEN?
            //static_assert(std::is_trivially_copyable<ble_hs_adv_fields>::value);

            std::memset(&gapAdvFields, 0, sizeof(gapAdvFields)); // undefined behaviour if struct not trivially copyable https://en.cppreference.com/w/cpp/string/byte/memset

            gapAdvFields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP; // "|" bitwise or operation. Meaning we add them together
            print_adv_field_flags(gapAdvFields);
          
            const unsigned int POWER_PRESENT = 1;
            // tx_pwr is the transmit power level of the devices radio signal  (not required to set btw)
            gapAdvFields.tx_pwr_lvl_is_present = POWER_PRESENT;
            gapAdvFields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO; // set the power level automatically
            print_adv_field_signal_power(gapAdvFields);

            name = ble_svc_gap_device_name();
            gapAdvFields.name = (uint8_t *)name;
            gapAdvFields.name_len = strlen(name);
            gapAdvFields.name_is_complete = 1;
        //
            //fields.uuids16 = (ble_uuid16_t[]) {
            //    BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID)
            //};
            //fields.num_uuids16 = 1;
            //fields.uuids16_is_complete = 1;
        //
            //rc = ble_gap_adv_set_fields(&fields);
            //if (rc != 0) {
            //    MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
            //    return;
            //}
        //
            ///* Begin advertising. */
            //memset(&adv_params, 0, sizeof adv_params);
            //adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
            //adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
            //rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
            //                       &adv_params, bleprph_gap_event, NULL);
            //if (rc != 0) {
            //    MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
            //    return;
            //}
        }


    } // namespace
   
    //CNimble::CNimble() 
    //{
    //   

    //    esp_err_t result = nimble_port_init(); //  Initialize controller and NimBLE host stack
    //    if (result != ESP_OK) {
    //        return;
    //    }

    //    ble_hs_cfg.reset_cb = nimble_on_reset_handle; // needs to be static why Christoffer teach me
    //    ble_hs_cfg.sync_cb = nimble_on_sync_handle; // callback when host and controller become synced
    //    // - Initilize host configurations before initilizing gap and gatt
    //    // - security manager is also configured before initilizing gap and gatt
    //    // - Order is important

    //    // set device name
    //    const char* p_DEVICE_NAME = "Chainsaw-server";
    //    result = ble_svc_gap_device_name_set(p_DEVICE_NAME);
    //    assert(result == 0);

    //    nimble_port_run(); // the callbacks will be called in here


    //    //ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    //    //ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
    //    //ble_hs_cfg.sm_io_cap = CONFIG_EXAMPLE_IO_TYPE;
    //    //ble_hs_cfg.sm_bonding = 1;
    //    //le_hs_cfg.sm_sc = 0;


    //    // we have NimBLE (Nordic's Bluetooth Low Energy) stack's store module that is used for:
    //    // provides a mechanism for persistent storage of various Bluetooth-related information.
    //    // This information typically includes security-related data, such as keys and pairing information, 
    //    // as well as other relevant configuration data.
    //    //ble_store_config_init();

    //}


    CNimble::CNimble(const char* deviceName) {

        esp_err_t result = nimble_port_init(); //  Initialize controller and NimBLE host stack
        if (result != ESP_OK) {
            return;
        }

        ble_hs_cfg.reset_cb = nimble_on_reset_handle; // needs to be static why Christoffer teach me
        ble_hs_cfg.sync_cb = nimble_on_sync_handle; // callback when host and controller become synced
        //ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
        //ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
        //ble_hs_cfg.sm_io_cap = CONFIG_EXAMPLE_IO_TYPE;
        //ble_hs_cfg.sm_bonding = 1;
        //le_hs_cfg.sm_sc = 0;

        // - Initilize host configurations before initilizing gap and gatt
        // - security manager is also configured before initilizing gap and gatt
        // - Order is important

        // set device name
        result = ble_svc_gap_device_name_set(deviceName);
        assert(result == 0);

        nimble_port_run(); // Note: the hs_cfg's callbacks are called in here


  


        // we have NimBLE (Nordic's Bluetooth Low Energy) stack's store module that is used for:
        // provides a mechanism for persistent storage of various Bluetooth-related information.
        // This information typically includes security-related data, such as keys and pairing information, 
        // as well as other relevant configuration data.
        //ble_store_config_init();
    }



} // namespace nimble



