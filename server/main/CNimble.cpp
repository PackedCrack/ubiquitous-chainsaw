#include "CNimble.h"

namespace nimble
{

    
    namespace 
    {


        const char* p_DEVICE_NAME = "Chainsaw-server";
        static uint8_t serverAddrType; // will be 1 or 0 depedning on rnd or pub addr
        #define SERVER_TAG "Chainsaw-server" // used for ESP_LOG
      

        // static  outside func is external linkage
      
        //void server_host_task(void* param) 
        //{
        //// call nimble_port_stop() // to exit from this func
        //ESP_LOGI(SERVER_TAG, "BLE Host Task Started");
        //nimble_port_run(); // Note: the hs_cfg's callbacks are called in here
        //ESP_LOGI(SERVER_TAG, "BLE Host Task Stopped");
        //nimble_port_freertos_deinit();
        //}

    void server_on_reset_handle(int reason) 
    {
        LOG_FATAL_FMT("ESP ERROR. Reset due to %d\n", reason);
    }

    void server_on_sync_handler(void) 
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

        if (serverAddrType == RND_ADDR) 
            ESP_LOGI(SERVER_TAG, "BLE Random Address: %02x:%02x:%02x:%02x:%02x:%02x", bleDeviceAddr[5], bleDeviceAddr[4], bleDeviceAddr[3], 
                                                                                    bleDeviceAddr[2], bleDeviceAddr[1], bleDeviceAddr[0]); 
        else if (serverAddrType == PUB_ADDR)
            LOG_INFO_FMT("BLE Public Address: %02x:%02x:%02x:%02x:%02x:%02x", bleDeviceAddr[5], bleDeviceAddr[4], bleDeviceAddr[3], 
                                                                                        bleDeviceAddr[2], bleDeviceAddr[1], bleDeviceAddr[0]);

  
        CGapService gap {p_DEVICE_NAME, serverAddrType};
        gap.advertise();
    }

    void server_gatt_svc_register_handle(struct ble_gatt_register_ctxt *ctxt, void *arg) 
    {
        // NIMBLE BLEPRPH EXAMPLE CODE
        char buf[BLE_UUID_STR_LEN];

        switch (ctxt->op) {
            case BLE_GATT_REGISTER_OP_SVC:
                ESP_LOGI(SERVER_TAG, "registered service %s with handle=%d\n",
                            ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf), ctxt->svc.handle);
                break;
            case BLE_GATT_REGISTER_OP_CHR:
                ESP_LOGI(SERVER_TAG,"registering characteristic %s with "
                            "def_handle=%d val_handle=%d\n",
                            ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf), ctxt->chr.def_handle, ctxt->chr.val_handle);
                break;
            case BLE_GATT_REGISTER_OP_DSC:
                ESP_LOGI(SERVER_TAG,"registering descriptor %s with handle=%d\n",
                            ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),ctxt->dsc.handle);
                break;
            default:
                assert(0);
                break;
        }
    }


    } // namespace
   

    CNimble::CNimble() 
    {
    
        esp_err_t result = nimble_port_init(); //  Initialize controller and NimBLE host stack
        if (result != ESP_OK) {
            return;
        }

        configure_nimble_stack();

        
        ble_svc_gap_init(); //register gap service to GATT server (service UUID 0x1800)
        ble_svc_gatt_init(); // register GATT service to GATT server0x1801
        //ble_svc_ans_init();  // register Alert Notification Service (ANS) to GATT server NOT NEEDED ON THIS SERVER

        nimble_port_run();

        // Using nimble in freertos does not work i get the error:
        // (E (1004) NimBLE: Host not enabled. Dropping the packet!) 
        // tried to have it in app_main() aswell but with the same result.
        // it works in the bleprph example. Even when i remove a lot of functionality they have

        // https://github.com/espressif/esp-idf/issues/3555
        // "If you want to use main task to run the NimBLE host then:
        // 1. Do not call nimble_port_freertos_init()
        // 2. At the end of your app_main() you can call nimble_port_run()"

        //nimble_port_freertos_init(server_host_task); 
    }

    void CNimble::configure_nimble_stack() 
    {
        int result;
        result = ble_svc_gap_device_name_set(p_DEVICE_NAME);
        assert(result == 0);

        ble_hs_cfg.reset_cb = server_on_reset_handle; 
        ble_hs_cfg.sync_cb = server_on_sync_handler; // entry point for starting advertising
        ble_hs_cfg.gatts_register_cb = server_gatt_svc_register_handle;

        //ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
        //ble_hs_cfg.sm_io_cap = CONFIG_EXAMPLE_IO_TYPE;
        //ble_hs_cfg.sm_bonding = 1;
        //le_hs_cfg.sm_sc = 0;
    }

    CNimble::~CNimble() 
    {

        int errorCode = nimble_port_stop();
        assert(errorCode == 0);

        esp_err_t result = nimble_port_deinit();
        assert(result == ESP_OK);
    }
} // namespace nimble



