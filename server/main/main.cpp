#include "sys/CSystem.hpp"
#include "ble/CNimble.hpp"

#include "sys/CNonVolatileStorage.hpp"

#include "esp_system.h"

#include <expected>
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
		std::expected<int, int> test{};
		print_chip_info();

		using namespace storage;

		storage::CNonVolatileStorage& nvs = storage::CNonVolatileStorage::instance();

		std::optional<CNonVolatileStorage::CReadWriter> realWriter = CNonVolatileStorage::CReadWriter::make_read_writer("STORAGE");
		if (!realWriter.has_value())
		{
			LOG_ERROR("Real CReader creation error");
		}
		else
		{
			LOG_INFO("Real CReader just works!");
			storage::CNonVolatileStorage::ReadBinaryResult readResult = realWriter.value().read_binary("BinaryData");
			if (readResult.code != NvsErrorCode::success)
			{
				LOG_ERROR("Error reading data using REAL CReader");
			}
			else
			{
				std::printf("Num bytes: %u\n", readResult.data.value().size() );
				for (uint8_t byte : readResult.data.value()) 
				{
        			std::printf("%02X ", byte);
    			}
				std::printf("\n");
			}
		}


		std::optional<CNonVolatileStorage::CReadWriter> writer = nvs.make_read_writer("STORAGE");
		if (!writer.has_value())
		{
			LOG_ERROR("Not real Creader creation error");
		}
		else	
		{
			LOG_INFO("Not real CReader just works!");
			storage::CNonVolatileStorage::ReadBinaryResult readResult = writer.value().read_binary("BinaryData");

			if (readResult.code != NvsErrorCode::success)
			{
				LOG_ERROR("Error reading data using Not Real CReader");
			}
			else
			{
				std::printf("Num bytes: %u\n", readResult.data.value().size() );
				for (uint8_t byte : readResult.data.value()) 
				{
        			std::printf("%02X ", byte);
    			}
				std::printf("\n");
			}
		}


		//auto a = security::CWolfCrypt::instance();

		// https://mynewt.apache.org/latest/

		//ble::CNimble nimble {};

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
