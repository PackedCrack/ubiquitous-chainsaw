#include "Nimble.hpp"


namespace ble
{

namespace 
{
//constexpr std::string_view SERVER_TAG {"Chainsaw-server"}; // used for ESP_LOG
std::promise<void> syncPromise {};
std::future<void> syncFuture = syncPromise.get_future();

auto gatt_service_register_event_handle = [](struct ble_gatt_register_ctxt *ctxt, void *arg) {
    // NIMBLE BLEPRPH EXAMPLE CODE
    char buf[BLE_UUID_STR_LEN];
    switch (ctxt->op) {
        case BLE_GATT_REGISTER_OP_SVC:
            LOG_INFO_FMT("registered service {} with handle={}\n",
                        ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf), ctxt->svc.handle);
            break;
        case BLE_GATT_REGISTER_OP_CHR:
            LOG_INFO_FMT("registering characteristic {} with "
                        "def_handle={} val_handle={}\n",
                        ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf), ctxt->chr.def_handle, ctxt->chr.val_handle);
            break;
        case BLE_GATT_REGISTER_OP_DSC:
            LOG_INFO_FMT("registering descriptor {} with handle={}\n",
                        ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),ctxt->dsc.handle);
            break;
        default:
            assert(0);
            break;
    }
};

auto make_on_sync_handle() 
{
    return [](){
        LOG_INFO("Ble Host and Controller have become synced!");
        syncPromise.set_value();
    };
}

auto make_on_reset_handle()
{
    return [](int reason){
        //https://mynewt.apache.org/latest/network/ble_setup/ble_sync_cb.html
        // HACK
        LOG_FATAL_FMT("Something went horribly wrong. Therefore we reset the whole device in order to make sure that main() is restarted. Reason={}", reason);
    };
}

auto make_host_task()
{
    return [](void* param){
        // call nimble_port_stop() // to exit from this func
        LOG_INFO("BLE Host Task Started");
        nimble_port_run();
    };
}


void configure_nimble_host()
{
    ble_hs_cfg.reset_cb = make_on_reset_handle(); 
    ble_hs_cfg.sync_cb = make_on_sync_handle();
    ble_hs_cfg.gatts_register_cb = gatt_service_register_event_handle;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr; // NOT THE BEST CALLBACK TO USE FOR PRODUCTION
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_bonding = 1u;
    ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC; // set flag, indicating that the local device is willing to share the encryption key (ENC) during pairing.
    ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC; // set flag, indicating that the remote device is expected to share the encryption key during pairing.
    ble_hs_cfg.sm_sc = 1u;
    //ble_store_config_init(); // which header is this?..
}



} // namespace


CNimble::CNimble()
    : m_gatt {}
    , m_gap {}
{
    esp_err_t result = nimble_port_init();
    if (result != ESP_OK) 
        throw std::runtime_error("An error occured during Construction of CNimble!");
    
    configure_nimble_host();
    m_gatt.register_services();
    nimble_port_freertos_init(make_host_task());
    syncFuture.get(); // Works without this, aslong as its faster :)
    m_gap.start();

}



//void CNimble::host_task(void* param)
//{
//    // call nimble_port_stop() // to exit from this func
//    LOG_INFO("BLE Host Task Started");
//    nimble_port_run();
//
//    LOG_INFO("BLE Host Task Stopped");
//    nimble_port_freertos_deinit();
//    nimble_port_deinit();
//}


} // namespace application