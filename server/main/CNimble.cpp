#include "CNimble.h"



namespace nimble
{

    namespace 
    {
        static void server_on_reset_cb(int reason) 
        {
            LOG_FATAL_FMT("ESP ERROR. Reset due to %d\n", reason); // something fucked up happend
        }

       static void nimble_on_sync_cb(void) 
       {

       }


    } // namespace
   

    CNimble::CNimble() 
    {
        std::printf("asdasd\nasdasdasd\nasdasdasd\n");
        esp_err_t errorCode = nimble_port_init(); //  Initialize controller and NimBLE host stack
        if (errorCode != ESP_OK) {
            return;
        }

        // - Initilize host configurations before initilizing gap and gatt
        // - security manager is also configured before initilizing gap and gatt
        // - Order is important

        ble_hs_cfg.reset_cb = nimble_on_reset_cb; // needs to be static why Christoffer teach me HOW TO TEST THIS
        ble_hs_cfg.sync_cb = nimble_on_sync_cb; // callback when host and controller become synced





      

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



