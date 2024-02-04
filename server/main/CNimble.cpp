#include "CNimble.h"



namespace nimble
{

    namespace 
    {

        #define SERVER_TAG "BLE_SERVER" // used for ESP_LOG


        // static variables
        static uint8_t serverAddrType;

        // Callback function
        static void nimble_on_reset_handle(int reason) 
        {
            LOG_FATAL_FMT("ESP ERROR. Reset due to %d\n", reason); // something fucked up happend
        }

       static void nimble_on_sync_handle(void) 
       {
            // Why is this code never triggered

            int returnCode;
            returnCode = ble_hs_util_ensure_addr(RND_ADDR); // TODO: 1 doenst work for some reason 
            assert(returnCode == 0);
//
            returnCode = ble_hs_id_infer_auto(0, &serverAddrType); // random address = private address? what does 0 mean? what is 1?
            std::printf("asd1\n %d\n", serverAddrType); // its a 1
            if (returnCode != 0) {
                LOG_FATAL_FMT("No address was able to be created for the server %d\n", returnCode); // todo how to log this properly?
                return;
            }
        //
            uint8_t addr_val[6] {0u};
            returnCode = ble_hs_id_copy_addr(serverAddrType, addr_val, NULL);
            if (returnCode != 0) 
            {
                LOG_WARN_FMT("Adress was unable to be retrieved %d\n", returnCode);
            }
                ESP_LOGE(SERVER_TAG, "Address is: \n");

            // u8p = addr;MODLOG_DFLT(INFO, "%02x:%02x:%02x:%02x:%02x:%02x", u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);

    
            //bleprph_advertise();
            
        }


    } // namespace
   

    CNimble::CNimble() 
    {
       

        std::printf("asdasd\nasdasdasd\nasdasdasd\n");
        esp_err_t errorCode = nimble_port_init(); //  Initialize controller and NimBLE host stack
        if (errorCode != ESP_OK) {
            return;
        }

        ble_hs_cfg.reset_cb = nimble_on_reset_handle; // needs to be static why Christoffer teach me
        ble_hs_cfg.sync_cb = nimble_on_sync_handle; // callback when host and controller become synced
        // - Initilize host configurations before initilizing gap and gatt
        // - security manager is also configured before initilizing gap and gatt
        // - Order is important

        nimble_port_run(); // the callbacks will be called in here





      

        //ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
        //ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
        //ble_hs_cfg.sm_io_cap = CONFIG_EXAMPLE_IO_TYPE;
        //ble_hs_cfg.sm_bonding = 1;
        //le_hs_cfg.sm_sc = 0;


        // we have NimBLE (Nordic's Bluetooth Low Energy) stack's store module that is used for:
        // provides a mechanism for persistent storage of various Bluetooth-related information.
        // This information typically includes security-related data, such as keys and pairing information, 
        // as well as other relevant configuration data.
        //ble_store_config_init();

        std::printf("asdasd\nasdasdasd\nasdasdasd\n");
    }




} // namespace nimble



