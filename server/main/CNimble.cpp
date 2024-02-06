#include "CNimble.h"
#include "CAdvertiseFields.h"
#include <bitset> // for visual testing

namespace nimble
{

    
    namespace 
    {


        const char* p_DEVICE_NAME = "Chainsaw-server";
        #define SERVER_TAG "Chainsaw-server" // used for ESP_LOG
        #define GATT_SVR_SVC_ALERT_UUID               0x1811 // https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Assigned_Numbers/out/en/Assigned_Numbers.pdf?v=1707124555335
        #define GATT_SVR_SVC_ATTRI_UUID                0x1801
        // static  outside func is external linkage
        static uint8_t serverAddrType; // will be 1 or 0 depedning on rnd or pub addr
      
        //void server_host_task(void* param) 
        //{
        //// call nimble_port_stop() // to exit from this func
        //ESP_LOGI(SERVER_TAG, "BLE Host Task Started");
        //nimble_port_run(); // Note: the hs_cfg's callbacks are called in here
        //ESP_LOGI(SERVER_TAG, "BLE Host Task Stopped");
        //nimble_port_freertos_deinit();
        //}

    static void server_on_reset_handle(int reason) 
    {
        LOG_FATAL_FMT("ESP ERROR. Reset due to %d\n", reason);
    }

    static void server_on_sync_handle(void) 
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
        result = ble_hs_id_copy_addr(serverAddrType, bleDeviceAddr, NULL); // serverAddrType is needed for advertisment packages (GAP)
        if (result != 0) 
        {
            LOG_FATAL_FMT("Adress was unable to be retrieved %d\n", result);
        }

        // TODO: HOW TO LOG INFO WITH PARAMS?
        if (serverAddrType == RND_ADDR) 
            ESP_LOGI(SERVER_TAG, "BLE Random Address: %02x:%02x:%02x:%02x:%02x:%02x", bleDeviceAddr[5], bleDeviceAddr[4], bleDeviceAddr[3], 
                                                                                    bleDeviceAddr[2], bleDeviceAddr[1], bleDeviceAddr[0]); 
        else if (serverAddrType == PUB_ADDR)
            LOG_INFO_FMT("BLE Public Address: %02x:%02x:%02x:%02x:%02x:%02x", bleDeviceAddr[5], bleDeviceAddr[4], bleDeviceAddr[3], 
                                                                                        bleDeviceAddr[2], bleDeviceAddr[1], bleDeviceAddr[0]);

        gap_advertise();
        
    }

    static int server_gap_on_connection_handler(struct ble_gap_event *event, void *arg) 
    {
        LOG_INFO("Server gap callback was triggered");
        return 0;
    }

    void gap_advertise()
    {
        
        int result;
        result = ble_svc_gap_device_name_set(p_DEVICE_NAME);
        assert(result == 0);

        CAdvertiseFields fields {p_DEVICE_NAME};
        
        result = ble_gap_adv_set_fields(&fields.data());
        if (result != 0) 
        LOG_FATAL_FMT("Error setting advertisement data! result = %d", result); // this will trigger if adv data exceeds adv packet size limit

        // Advertising parameters
        ble_gap_adv_params gapAdvConfigParams;
        std::memset(&gapAdvConfigParams, 0, sizeof(gapAdvConfigParams));
        gapAdvConfigParams.conn_mode = BLE_GAP_CONN_MODE_UND;
        gapAdvConfigParams.disc_mode = BLE_GAP_DISC_MODE_GEN;

        result = ble_gap_adv_start(serverAddrType, NULL, BLE_HS_FOREVER, &gapAdvConfigParams, server_gap_on_connection_handler, NULL);
        if (result != 0) {
            LOG_FATAL_FMT("Error starting advertisement = %d", result);
            return;
        }
    }

    } // namespace
   

    CNimble::CNimble() 
    {
    
        esp_err_t result = nimble_port_init(); //  Initialize controller and NimBLE host stack
        if (result != ESP_OK) {
            return;
        }

        ble_hs_cfg.reset_cb = server_on_reset_handle; // needs to be static why Christoffer teach me
        ble_hs_cfg.sync_cb = server_on_sync_handle; // callback when host and controller become synced
        //ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
        //ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
        //ble_hs_cfg.sm_io_cap = CONFIG_EXAMPLE_IO_TYPE;
        //ble_hs_cfg.sm_bonding = 1;
        //le_hs_cfg.sm_sc = 0;

        // - Initilize host configurations before initilizing gap and gatt
        // - security manager is also configured before initilizing gap and gatt
        // - Order is important

        // set device name



        ble_svc_gap_init();
        nimble_port_run();
        //nimble_port_freertos_init(server_host_task); // does not work for some reason (E (1004) NimBLE: Host not enabled. Dropping the packet!) maybe becasue undefined?
        //nimble_port_freertos_init();

        // we have NimBLE (Nordic's Bluetooth Low Energy) stack's store module that is used for:
        // provides a mechanism for persistent storage of various Bluetooth-related information.
        // This information typically includes security-related data, such as keys and pairing information, 
        // as well as other relevant configuration data.
        //ble_store_config_init();
    }

 

   


    CNimble::~CNimble() 
    {

        int errorCode = nimble_port_stop();
        assert(errorCode == 0);

        esp_err_t result = nimble_port_deinit();
        assert(result == ESP_OK);
    }
} // namespace nimble



