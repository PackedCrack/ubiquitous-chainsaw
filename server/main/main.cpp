#include "CChip.hpp"
#include "Nimble.hpp"

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
	
	try 
	{
        ble::CNimble nimble {};
		//ble::CNimble nimble1 {}; // will fuck up the advertising aswell
    } catch (const std::exception& error) {
		LOG_ERROR_FMT("Caught exception: {}", error.what());
    } catch (...) {
		LOG_ERROR("Caught unknown exception");
    }

	while (true)
	{
		//chainsawServer.rssi();
		// Perform any periodic tasks here
		vTaskDelay(pdMS_TO_TICKS(3000)); // milisecs
	}
}
