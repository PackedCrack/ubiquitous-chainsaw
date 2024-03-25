#include "CNimble.hpp"
#include "profiles/CWhoAmI.hpp"
#include "profiles/CProfileCacheBuilder.hpp"
// esp
#include "esp_bt.h"

#include <iostream>

namespace 
{
auto make_callback_gatt_service_register()
{
	return [](ble_gatt_register_ctxt* ctxt, void* arg)
	{
		std::array<char, BLE_UUID_STR_LEN> buffer{};
    	switch (ctxt->op) 
		{
    	    case BLE_GATT_REGISTER_OP_SVC:
    	        LOG_INFO_FMT("Registered service = \"{}\" with handle = \"{}\"",
    	                    ble_uuid_to_str(ctxt->svc.svc_def->uuid, buffer.data()), ctxt->svc.handle);
    	        return;
    	    case BLE_GATT_REGISTER_OP_CHR:
    	        LOG_INFO_FMT("Registered characteristic: \"{}\" with "
    	                    "def_handle = \"{}\" val_handle = \"{}\"",
    	                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buffer.data()), ctxt->chr.def_handle, ctxt->chr.val_handle);
    	        return;
    	    case BLE_GATT_REGISTER_OP_DSC:
    	        LOG_INFO_FMT("Registered descriptor = \"{}\" with handle = {}",
    	                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buffer.data()),ctxt->dsc.handle);
    	        return;
    	}

		__builtin_unreachable();
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
	// retarded global that all of the example code uses - the api is like this dont blame us.
    ble_hs_cfg.reset_cb = make_on_reset_handle(); 
    ble_hs_cfg.sync_cb = ble::CNimble::sync_callback;

	#ifndef NDEBUG
    ble_hs_cfg.gatts_register_cb = make_callback_gatt_service_register();
	#else
	ble_hs_cfg.gatts_register_cb = nullptr;
	#endif

    ble_hs_cfg.store_status_cb = ble_store_util_status_rr; // NOT THE BEST CALLBACK TO USE FOR PRODUCTION
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_bonding = 1u;
    ble_hs_cfg.sm_our_key_dist = ble_hs_cfg.sm_our_key_dist | BLE_SM_PAIR_KEY_DIST_ENC; // set flag, indicating that the local device is willing to share the encryption key (ENC) during pairing.
    ble_hs_cfg.sm_their_key_dist = ble_hs_cfg.sm_our_key_dist | BLE_SM_PAIR_KEY_DIST_ENC; // set flag, indicating that the remote device is expected to share the encryption key during pairing.
    ble_hs_cfg.sm_sc = 1u;
    //ble_store_config_init(); // which header is this?..
}
void print_ble_address()
{
    std::array<uint8_t, 6> bleDeviceAddr {};
    ble::NimbleErrorCode result = ble::NimbleErrorCode{ 
		ble_hs_id_copy_addr(static_cast<uint8_t>(ble::AddressType::randomMac), bleDeviceAddr.data(), nullptr) };

    if (result != ble::NimbleErrorCode::success) 
        LOG_FATAL_FMT("Adress was unable to be retreived {}", ble::nimble_error_to_string(result));

    std::printf("BLE Device Address: %02x:%02x:%02x:%02x:%02x:%02x \n", 
				bleDeviceAddr[5], 
				bleDeviceAddr[4],
				bleDeviceAddr[3],
				bleDeviceAddr[2],
				bleDeviceAddr[1],
				bleDeviceAddr[0]);
}
[[nodiscard]] ble::AddressType generate_random_device_address() 
{
	static constexpr bool PREFER_RANDOM = true;
	static constexpr bool USE_PRIVATE_ADDR = false;

    uint8_t expectedAddrType = ble::INVALID_ADDRESS_TYPE;
    auto result = ble::NimbleErrorCode{ ble_hs_util_ensure_addr(PREFER_RANDOM) };
    if (result != ble::NimbleErrorCode::success)
	{
		LOG_FATAL_FMT("No address was able to be ensured ERROR={}", ble::nimble_error_to_string(result));
	}
        
    result = ble::NimbleErrorCode{ ble_hs_id_infer_auto(USE_PRIVATE_ADDR, &expectedAddrType) }; // 1/private do not work here, type will depend ble_hs_util_ensure_addr()
    if (result != ble::NimbleErrorCode::success)
	{
		LOG_FATAL_FMT("No address was able to be inferred ERROR={}", ble::nimble_error_to_string(result));
	}

    ASSERT(expectedAddrType == static_cast<uint8_t>(ble::AddressType::randomMac), "Assigned wrong bluetooth address type");
	#ifndef NDEBUG
    print_ble_address();
	#endif

    return ble::AddressType{ expectedAddrType };
}
} // namespace
namespace ble
{
std::pair<std::shared_ptr<std::mutex>, std::shared_ptr<std::condition_variable>> CNimble::synchronization_primitives()
{
	static std::mutex m{};
	std::lock_guard lock{ m };

	static auto pMutex = std::make_shared<std::mutex>();
	static auto pCV = std::make_shared<std::condition_variable>();

	return std::pair<std::shared_ptr<std::mutex>, std::shared_ptr<std::condition_variable>>{ pMutex, pCV };
}
void CNimble::sync_callback()
{
	auto[pMutex, pCV] = synchronization_primitives();
	std::unique_lock lock{ *pMutex };
	pCV->notify_one();
}
CNimble::CNimble()
	: m_pGap{ nullptr }
	, m_pProfileCache{ nullptr }
{
	// https://mynewt.apache.org/latest/network/ble_setup/ble_addr.html#method-3-configure-a-random-address-at-runtime
    esp_err_t nimbleResult = nimble_port_init();
    if (!success(nimbleResult))
    {
        if (nimbleResult != ESP_ERR_INVALID_STATE)
		{
			LOG_FATAL("Error initilizing nimble_port_init() due to controller is not idle");
		}
		else
		{
			LOG_FATAL("CNimble constructor failed. invalid state");
		}
    }


	auto[pMutex, pCV] = synchronization_primitives();
	std::unique_lock lock{ *pMutex };

    configure_nimble_host();
	CProfileCache cache = CProfileCacheBuilder()
							.add_whoami()
							.build();
	m_pProfileCache = std::make_unique<CProfileCache>(std::move(cache));


    nimble_port_freertos_init(make_host_task());
	pCV->wait(lock);

	AddressType macType = generate_random_device_address();
	m_pGap = std::make_unique<CGap>(macType);
}
CNimble::~CNimble()
{
    std::printf("CNimble destructor\n");
    //esp_restart();

	// destructor order -> CNimble -> Gap -> Gatt
    // since we use deinit in CNimble destructor, the destructors of Gap and Gatt will crash..

    //int result = m_gap.drop_connection(BLE_HS_ENOENT);   

	if(m_pGap)
		m_pGap.release();

	if(m_pProfileCache)
		m_pProfileCache.release();

    //std::optional<CGap::Error> result = m_gap.end_advertise();
    //if (result != std::nullopt)
    //{
    //    LOG_FATAL_FMT("{}", result.value().msg.c_str());
    //}

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
} // namespace ble