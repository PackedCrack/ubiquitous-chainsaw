#include "CNimble.hpp"
#include "profiles/CWhoAmI.hpp"
#include "esp_bt.h"


namespace 
{
//constexpr std::string_view SERVER_TAG {"Chainsaw-server"}; // used for ESP_LOG
//std::promise<void> syncPromise {};
//std::future<void> syncFuture = syncPromise.get_future();
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
        //syncPromise.set_value();
    };
}
auto make_on_reset_handle()
{
    return [](int reason){
        //https://mynewt.apache.org/latest/network/ble_setup/ble_sync_cb.html
        LOG_ERROR_FMT("Something went horribly wrong. Therefore we reset the whole device in order to make sure that main() is restarted. Reason={}", reason);
        //syncPromise = std::promise<void>{};
        //syncFuture = syncPromise.get_future();
    };
}
auto make_host_task()
{
    return [](void* param){
        // call nimble_port_stop() // to exit from this func
        LOG_INFO("BLE Host Task Started");
        nimble_port_run();
        std::printf("exited from nimble_port_run()\n");
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


namespace ble
{
CNimble::CNimble()
    : //m_gatt {}
     m_gap {}
{
	auto[value, error] = CWhoAmI::make_whoami();


	Result<CWhoAmI, CWhoAmI::Error> result2 = CWhoAmI::make_whoami();
	if(!(result2.value))
	{
		LOG_FATAL("Failed to instantiate whoami profile");
	}
	// else
	CWhoAmI whoami{ std::move(result2.value.value()) };


    esp_err_t nimbleResult = nimble_port_init();
    std::printf("Result: %d\n", nimbleResult);
    if (nimbleResult != SUCCESS)
    {
        if (nimbleResult != ESP_ERR_INVALID_STATE)
            throw std::runtime_error("Error initilizing nimble_port_init() due to controller is not idle");
    }

    configure_nimble_host();
    //result = m_gatt.register_services();
    //if (result != 0)
    //{
    //    if (result == BLE_HS_EBUSY)
    //        throw std::runtime_error("GATT server could not be reset due to existing connections or active GAP procedures");

    //    if (result == BLE_HS_EINVAL)
    //        throw std::runtime_error("Services array contains an invalid resource definition");
    
    //    if (result == BLE_HS_ENOMEM)
    //        throw std::runtime_error("heap exhaustion");

    //    throw std::runtime_error("Unknown error");
    //}

  
    nimble_port_freertos_init(make_host_task());

    //syncFuture.get();

    std::optional<CGap::Error> result = m_gap.start();
    if (result != std::nullopt)
    {
        CGap::Error err = *result; // bugg
        std::printf(err.msg.c_str());
        throw std::runtime_error(err.msg);
    }
}
CNimble::~CNimble()
{
    std::printf("CNimble destructor\n");
    //esp_restart();

	// destructor order -> CNimble -> Gap -> Gatt
    // since we use deinit in CNimble destructor, the destructors of Gap and Gatt will crash..

    //int result = m_gap.drop_connection(BLE_HS_ENOENT);   

    std::optional<CGap::Error> result = m_gap.end_advertise();
    if (result != std::nullopt)
    {
        // handle error here
    }

    //result = ble_gatts_reset(); // TODO MAKE AS A FUNC IN CGATT
    //ASSERT(result == SUCCESS, "Error unable to reset CGatt due to existing connections or active GAP procedures!");

    nimble_port_freertos_deinit();
    //result = nimble_port_deinit();
    //ASSERT(result == SUCCESS, "Error unable to deinit nimble port!")

    //syncPromise = std::promise<void>{};
    //syncFuture = syncPromise.get_future();

    /*   Notes
    Calling nimble_port_stop(); --> E (1092) FreeRTOS: FreeRTOS Task "nimble_host" should not return, Aborting now!

    If i use nimble_port_deinit();
    I get no init "error" when initiliziting again
    but i do get and error (530=BLE_ERR_INV_HCI_CMD_PARMS) when trying to start advertising again.

    If i dont use nimble_port_deinit();
    I get an init error (279=ESP_ERR_INVALID_STATE) when trying to initilize again
    But i can now advertise

    I dont know what is causing the 530 error code.
    */
}
CNimble::CNimble(CNimble&& other) noexcept
    : //m_gatt { std::move(other.m_gatt) }
     m_gap { std::move(other.m_gap) } 
{
    // no pointers have been moved
}
CNimble& CNimble::operator=(CNimble&& other)
{
    /*
        1. Clean up all visible resources
        2. Transfer the content of other into this
        3. Leave other in a valid but undefined state
    */
    
    // Check if other exists?
    //m_gatt = std::move(other.m_gatt);
    m_gap = std::move(other.m_gap);
    return *this;
}
} // namespace ble