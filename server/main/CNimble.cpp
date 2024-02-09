#include "CNimble.hpp"



namespace nimble
{

    
namespace 
{
    //const std::string_view deviceName {"Chainsaw-server"};
    //uint8_t serverAddrType;
    bool isBleInitilized = false;
    constexpr std::string_view SERVER_TAG {"Chainsaw-server"}; // used for ESP_LOG


void ble_on_sync_handler(void) 
{
    //ble_generate_random_device_address();
    isBleInitilized = true;
}




} // namesapce

CNimble::CNimble() 
{

    esp_err_t result = nimble_port_init(); //  will fail if called twice
    if (result != ESP_OK) {
        return;
    }

    configure_nimble_host();
}


void CNimble::configure_nimble_host() 
{
    ble_hs_cfg.reset_cb = ble_on_reset_event_handle; 
    ble_hs_cfg.sync_cb = ble_on_sync_handler; // entry point for starting advertising
    ble_hs_cfg.gatts_register_cb = ble_gatt_service_register_event_handle;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr; // NOT THE BEST CALLBACK TO USE FOR PRODUCTION
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_bonding = 1u;
    ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC; // set flag, indicating that the local device is willing to share the encryption key (ENC) during pairing.
    ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC; // set flag, indicating that the remote device is expected to share the encryption key during pairing.
    ble_hs_cfg.sm_sc = 1u;
    //ble_store_config_init(); // which header is this?..
}


bool CNimble::isInitilized() 
    { return isBleInitilized; }


void CNimble::task(void* param)
{
    // call nimble_port_stop() // to exit from this func
    LOG_INFO("BLE Host Task Started");
    nimble_port_run(); // Note: the hs_cfg's callbacks are called in here
    LOG_INFO("BLE Host Task Stopped");
    nimble_port_freertos_deinit();
}


void CNimble::ble_gatt_service_register_event_handle(struct ble_gatt_register_ctxt *ctxt, void *arg) 
{
    // TODO: MOVE THSI TO GATT SERVER
    // NIMBLE BLEPRPH EXAMPLE CODE
    char buf[BLE_UUID_STR_LEN];
    switch (ctxt->op) {
        case BLE_GATT_REGISTER_OP_SVC:
            ESP_LOGI(SERVER_TAG.data(), "registered service %s with handle=%d\n",
                        ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf), ctxt->svc.handle);
            break;
        case BLE_GATT_REGISTER_OP_CHR:
            ESP_LOGI(SERVER_TAG.data(),"registering characteristic %s with "
                        "def_handle=%d val_handle=%d\n",
                        ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf), ctxt->chr.def_handle, ctxt->chr.val_handle);
            break;
        case BLE_GATT_REGISTER_OP_DSC:
            ESP_LOGI(SERVER_TAG.data(),"registering descriptor %s with handle=%d\n",
                        ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),ctxt->dsc.handle);
            break;
        default:
            assert(0);
            break;
    }
}


void CNimble::ble_on_reset_event_handle(int reason) 
{
    // https://mynewt.apache.org/latest/network/ble_setup/ble_sync_cb.html
    // possible bug area, has to be tested so that the reinitilization is done correctly
    isBleInitilized = false;
    //LOG_FATAL_FMT("ESP ERROR. Reset due to %d\n", reason);
    std::printf("RESET CALLBACK WAS CALLED!\n");
    std::printf("RESET CALLBACK WAS CALLED!\n");
    std::printf("RESET CALLBACK WAS CALLED!\n");
    /*
    In the NimBLE stack, the reset_cb callback is invoked when a soft reset event occurs. 
    A soft reset is typically triggered by application code or stack internals and does not necessarily indicate a crash. 
    For instance, the reset_cb might be called in response to a call to ble_hs_reset() or when the stack detects a configuration change requiring a reset.

    If the system crashes due to a watchdog timeout, buffer overflow, or other low-level issues, it's unlikely that the reset_cb callback will be called.


    In the case of a hard reset, such as a watchdog reset or a system crash, the behavior can vary. 
    Some systems may automatically restart the application after a hard reset, in which case app_main() 
    would be called again during the startup process. However, in other systems, a hard reset may result in a 
    complete system restart where the firmware reboots without automatically restarting the application.


    TESTED: This callback is not called when buffer overflow occurs! the device will instead reboot and call app_main() again
    TESTED: Not called when pressing the hardware reset button

    */
}


CNimble::~CNimble() 
 {
     std::printf("entered deconstrtuctor of CNimble");

     //int errorCode = nimble_port_stop();
     //assert(errorCode == 0);

     //esp_err_t result = nimble_port_deinit();
     //assert(result == ESP_OK);
 }
} // namespace nimble



