#include "Nimble.hpp"


namespace nimble
{

    
namespace 
{

constexpr std::string_view SERVER_TAG {"Chainsaw-server"}; // used for ESP_LOG
std::promise<void> syncPromise {};
std::future<void> syncFuture = syncPromise.get_future();

auto on_sync_event_handler = []() {
    syncPromise.set_value();
    LOG_INFO("BLE host/controller has become synced");
};


auto on_reset_event_handle = [](int reason) {
    LOG_INFO("BLE HOST SOFT RESET TRIGGERED");
};


auto gatt_service_register_event_handle = [](struct ble_gatt_register_ctxt *ctxt, void *arg) {
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
};


} // namespace

void configure_nimble_host() 
{
    esp_err_t result = nimble_port_init(); //  will fail if called twice
    if (result != ESP_OK) {
        return;
    }

    ble_hs_cfg.reset_cb = on_reset_event_handle; 
    ble_hs_cfg.sync_cb = on_sync_event_handler;
    ble_hs_cfg.gatts_register_cb = gatt_service_register_event_handle;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr; // NOT THE BEST CALLBACK TO USE FOR PRODUCTION
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_bonding = 1u;
    ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC; // set flag, indicating that the local device is willing to share the encryption key (ENC) during pairing.
    ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC; // set flag, indicating that the remote device is expected to share the encryption key during pairing.
    ble_hs_cfg.sm_sc = 1u;
    //ble_store_config_init(); // which header is this?..
}

void nimble_host_task(void* param)
{
    // call nimble_port_stop() // to exit from this func
    LOG_INFO("BLE Host Task Started");
    nimble_port_run();

    LOG_INFO("BLE Host Task Stopped");
    nimble_port_freertos_deinit();
    nimble_port_deinit();
}

void wait_for_sync()
{ 
    syncFuture.get();
}


//CNimble::CNimble() 
//{
//    esp_err_t result = nimble_port_init(); //  will fail if called twice
//    if (result != ESP_OK) {
//        return;
//    }
//
//    configure_nimble_host();
//}
//
//
//void CNimble::wait_for_sync()
//{
//	sync_future.get();
//}
//
//
//void CNimble::configure_nimble_host() 
//{
//    ble_hs_cfg.reset_cb = on_reset_event_handle; 
//    ble_hs_cfg.sync_cb = on_sync_event_handler;
//    ble_hs_cfg.gatts_register_cb = gatt_service_register_event_handle;
//    ble_hs_cfg.store_status_cb = ble_store_util_status_rr; // NOT THE BEST CALLBACK TO USE FOR PRODUCTION
//    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
//    ble_hs_cfg.sm_bonding = 1u;
//    ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC; // set flag, indicating that the local device is willing to share the encryption key (ENC) during pairing.
//    ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC; // set flag, indicating that the remote device is expected to share the encryption key during pairing.
//    ble_hs_cfg.sm_sc = 1u;
//    //ble_store_config_init(); // which header is this?..
//}
//
//
//void CNimble::task(void* param)
//{
//    // call nimble_port_stop() // to exit from this func
//    LOG_INFO("BLE Host Task Started");
//    nimble_port_run(); // Note: the hs_cfg's callbacks are called in here
//    LOG_INFO("BLE Host Task Stopped");
//    nimble_port_freertos_deinit();
//}
//

//CNimble::~CNimble() 
// {
//     std::printf("entered deconstrtuctor of CNimble");
//
//     //int errorCode = nimble_port_stop();
//     //assert(errorCode == 0);
//
//     //esp_err_t result = nimble_port_deinit();
//     //assert(result == ESP_OK);
// }
} // namespace nimble



