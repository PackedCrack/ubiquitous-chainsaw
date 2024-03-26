#include "sys/CSystem.hpp"
#include "ble/CNimble.hpp"

#include "sys/CNonVolatileStorage.hpp"


#include "../../client/src/security/CWolfCrypt.hpp"


void print_chip_info()
{
	sys::CSystem system{};
	sys::CChip chip = system.chip_info();

	std::printf("\nChip information");
	std::printf("\nRevision: %s", chip.revision().c_str());
	std::printf("\nNumber of Cores: %i", chip.cores());
	std::printf("\nFlash Memory: %s", chip.embedded_psram() ? "Embedded" : "External");
	std::printf("\nPSRAM: %s", chip.embedded_flash_memory() ? "Embedded" : "External");
	std::printf("\nSupports Wifi 2.4ghz: %s", chip.wifi() ? "True" : "False");
	std::printf("\nSupports Bluetooth LE: %s", chip.bluetooth_le() ? "True" : "False");
	std::printf("\nSupports Bluetooth Classic: %s", chip.bluetooth_classic() ? "True" : "False");
	std::printf("\nSupports IEEE 802.15.4: %s", chip.IEEE_802_15_4() ? "True" : "False");
	std::printf("\nCurrent minimum free heap: %lu bytes\n\n", system.min_free_heap());
}


extern "C" void app_main(void)
{
	sys::CSystem system{};
	try 
	{
		print_chip_info();
		storage::CNonVolatileStorage nvs{};

		auto a = security::CWolfCrypt::instance();


		// https://mynewt.apache.org/latest/

		//{
        //ble::CNimble tmp {};
		//}
		ble::CNimble nimble {};
		//ble::CNimble nimble {std::move(tmp)};
		//ble::CNimble nimble = std::move(tmp);

		while (true)
		{
			//chainsawServer.rssi();
			// Perform any periodic tasks here
			vTaskDelay(pdMS_TO_TICKS(3000)); // milisecs
		}
    } 
	catch (const exception::fatal_error& error) 
	{
		LOG_ERROR_FMT("Caught exception: {}", error.what());

		// TODO:: we should do more than restart here
		system.restart();
    }
}
