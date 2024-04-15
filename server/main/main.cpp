#include "sys/CSystem.hpp"
#include "ble/CNimble.hpp"
#include "sys/CNonVolatileStorage.hpp"

#include "security/CWolfCrypt.hpp"
#include "security/CHash.hpp"
#include "security/sha.hpp"
#include "security/CWolfCrypt.hpp"
#include "security/CRandom.hpp"
#include "security/ecc_key.hpp"

#include "esp_system.h"

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
	std::printf("\nCurrent minimum free heap: %u bytes\n\n", static_cast<unsigned int>(system.min_free_heap()));
}

extern "C" void app_main(void)
{
	sys::CSystem system{};
	try 
	{

		
		//std::expected<int, int> test{};
		//print_chip_info();

		using namespace storage;

		//[[maybe_unused]] const CNonVolatileStorage& nvs = CNonVolatileStorage::instance();
		
		auto crypto = security::CWolfCrypt::instance();
		if(crypto)
		{
			std::printf("Works\n");
		}


		//ble::CNimble nimble{};



		while (true)
		{
			//chainsawServer.rssi();
			// Perform any periodic tasks here
			
			vTaskDelay(pdMS_TO_TICKS(3000)); // milisecs
			//std::printf("taskdelay\n");

		}
    } 
	catch (const exception::fatal_error& error) 
	{
		LOG_ERROR_FMT("Caught exception: {}", error.what());

		// TODO:: we should do more than restart here
		system.restart();
    }
}
