#include "CChip.hpp"
#include "CNimble.hpp"
#include "CChainsaw.hpp"

#include "CNonVolatileStorage.hpp"




void print_chip_info()
{
	sys::CChip chip{};

	std::printf("\nChip information");
	std::printf("\nRevision: %s", chip.revision().c_str());
	std::printf("\nNumber of Cores: %i", chip.cores());
	std::printf("\nFlash Memory: %s", chip.embedded_psram() ? "Embedded" : "External");
	std::printf("\nPSRAM: %s", chip.embedded_flash_memory() ? "Embedded" : "External");
	std::printf("\nSupports Wifi 2.4ghz: %s", chip.wifi() ? "True" : "False");
	std::printf("\nSupports Bluetooth LE: %s", chip.bluetooth_le() ? "True" : "False");
	std::printf("\nSupports Bluetooth Classic: %s", chip.bluetooth_classic() ? "True" : "False");
	std::printf("\nSupports IEEE 802.15.4: %s", chip.IEEE_802_15_4() ? "True" : "False");
	std::printf("\nCurrent minimum free heap: %lu bytes\n\n", sys::min_free_heap());
}

extern "C" void app_main(void)

{
	print_chip_info();
	storage::CNonVolatileStorage nvs{};

	// https://mynewt.apache.org/latest/

	// dont want nmible to "own" our server
	// our server runs along side it / ontop of it
	// but nimble should live longer than our server
	// what happens to our server if nimble crashes?
	// app_main() will be called


	// Initilize nimble host
	// configre the on sync/reset callbacks
	// configure Security Manager
	// configure Store
	// configure optional callback that gets triggered when a service is added (good during development) 
	nimble::CNimble ble{};

	// initilize chainsaw server
	// Initilize Gap service and Gatt service ->     ble_svc_gap_init();     ble_svc_gatt_init();
	// then add the other GATT services
	application::CChainsaw chainsaw{};


	nimble_port_freertos_init(ble.task);

	// sync callback will now be called
	// bluetooth device address will be created
	// bool that synchronization will be set
	

	bool synced = false;
	while (!synced)
	{
		synced = ble.isInitilized();
	}

	chainsaw.start();

	while (true)
	{
		// Perform any periodic tasks here

		vTaskDelay(pdMS_TO_TICKS(5000)); // milisecs
	}
}
