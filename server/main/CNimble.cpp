#include "CNimble.hpp"

namespace nimble
{

    
namespace 
{
    const std::string_view deviceName {"Chainsaw-server"};
    uint8_t serverAddrType;
    bool isBleInitilized = false;
    constexpr std::string_view SERVER_TAG {"Chainsaw-server"}; // used for ESP_LOG


void ble_generate_random_device_address() 
{
    int result;
    const int RND_ADDR = 1;
    const int PUB_ADDR = 0;
    result = ble_hs_util_ensure_addr(RND_ADDR);
    assert(result == 0);
    result = ble_hs_id_infer_auto(PUB_ADDR, &serverAddrType); // 1/private do not work here, type will depend ble_hs_util_ensure_addr()
    if (result != 0) {
        LOG_FATAL_FMT("No address was able to be inferred %d\n", result);
    }

    std::array<uint8_t, 6> bleDeviceAddr {};
    result = ble_hs_id_copy_addr(serverAddrType, bleDeviceAddr.data(), NULL); // serverAddrType is needed for advertisment packages (GAP)
    if (result != 0) 
        LOG_FATAL_FMT("Adress was unable to be assigned %d\n", result);

    std::printf("BLE Device Address: %02x:%02x:%02x:%02x:%02x:%02x \n", bleDeviceAddr[5],bleDeviceAddr[4],bleDeviceAddr[3],bleDeviceAddr[2],bleDeviceAddr[1],bleDeviceAddr[0]);
    //LOG_INFO_FMT("BLE Device Address: {}:{}:{}:{}:{}:{}", bleDeviceAddr[5], bleDeviceAddr[4], bleDeviceAddr[3], 
    //                                                                            bleDeviceAddr[2], bleDeviceAddr[1], bleDeviceAddr[0]);
}


void ble_on_sync_handler(void) 
{
    ble_generate_random_device_address(); // do we need to store this?
    isBleInitilized = true;
}


} // namesapce


CNimble::CNimble() 
//: m_initilized{false}
{
    // m_gattServer constructor is also called
    // the object is constructed but not in a valid state
    // the reason being that m_gattServer can not be started before host/controller becomes synced

    esp_err_t result = nimble_port_init(); //  will fail if called twice
    if (result != ESP_OK) {
        return;
    }


    configure_nimble_host();
}


[[NoDiscard]] uint8_t CNimble::gap_param_is_alive()
{
    return m_gattServer.gap_param_is_alive();
}

void CNimble::start()
{ 
    // welcome to bug central
    assert(isBleInitilized);

    // dont question it :) (damn RAII) // dev error code, not for production
    uint8_t paramValueBefore = gap_param_is_alive();
	assert(paramValueBefore != 2);

    std::printf("Move assignment of CGattServer into m_gattServer\n");
    m_gattServer = {deviceName, serverAddrType};  // Custom constructor -> move assignment -> De-construtor

    uint8_t paramValueAfter = gap_param_is_alive();
	assert(paramValueAfter == 2);
} 


bool CNimble::isInitilized() 
    { return isBleInitilized; }


void CNimble::task(void* param)
{
    std::printf("ble.task() Entered server_host_task\n");
    // call nimble_port_stop() // to exit from this func
    LOG_INFO("BLE Host Task Started");
    nimble_port_run(); // Note: the hs_cfg's callbacks are called in here
    LOG_INFO("BLE Host Task Stopped");
    nimble_port_freertos_deinit();
    
}


void CNimble::configure_nimble_host() 
{
    int result;
    result = ble_svc_gap_device_name_set(deviceName.data());
    assert(result == 0);
    ble_hs_cfg.reset_cb = ble_on_reset_event_handle; 
    ble_hs_cfg.sync_cb = ble_on_sync_handler; // entry point for starting advertising
    ble_hs_cfg.gatts_register_cb = ble_gatt_service_register_event_handle;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr; // handles status updates related to NVS
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_bonding = 1u;
    ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC; // set flag, indicating that the local device is willing to share the encryption key (ENC) during pairing.
    ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC; // set flag, indicating that the remote device is expected to share the encryption key during pairing.
    ble_hs_cfg.sm_sc = 1u;
    // do these need to be called again?
    ble_svc_gap_init(); //register gap service to GATT server (service UUID 0x1800)
    ble_svc_gatt_init(); // register GATT service to GATT server (service UUID 0x1801)
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
    LOG_FATAL_FMT("ESP ERROR. Reset due to %d\n", reason);
}


CNimble::~CNimble() 
 {
     std::printf("entered deconstrtuctor of CNimble");

     int errorCode = nimble_port_stop();
     assert(errorCode == 0);

     esp_err_t result = nimble_port_deinit();
     assert(result == ESP_OK);
 }
} // namespace nimble



