#include "CChainsaw.hpp"


namespace application
{ 

namespace 
{

// TODO: They are only used once, but are still alive rest of the program..
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
            LOG_INFO_FMT("registered service {} with handle={}\n",
                        ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf), ctxt->svc.handle);
            break;
        case BLE_GATT_REGISTER_OP_CHR:
            LOG_INFO_FMT("registering characteristic {} with " "def_handle={} val_handle={}\n",
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

} // namespace


void CChainsaw::rssi()
{
    m_gap.rssi();
}


CChainsaw::CChainsaw()
    : m_nimbleHost {} // initilize / configure callbacks
    , m_gap {} // maybe gap should own Gatt since gap also uses gatt?
    , m_gatt {}
{

    m_nimbleHost.start();
    m_gap.start();
}

} // namespace application