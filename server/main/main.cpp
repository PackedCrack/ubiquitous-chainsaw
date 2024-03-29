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

		CNonVolatileStorage nvs{};

		// WRITE AND READ EXAMPLE USAGE
		// TODO where should we store namespace names and keys? in Storage:: as a enum?
		std::optional<CNonVolatileStorage::CReadWriter> writer = nvs.make_read_writer("STORAGE");
		if (!writer.has_value())
		{
			LOG_ERROR("CReadWriter creation error");
		}
		std::vector<uint8_t> newData {0x5,0x5,0x5,0x5,0x5,0x5};
		CNonVolatileStorage::WriteResult writeResult = writer.value().write_binary("BinaryData", newData);
		if (writeResult.code != NvsErrorCode::success)
		{
			LOG_ERROR("Something went wrong when writing to NVS");
		}
		std::optional<CNonVolatileStorage::CReader> reader = nvs.make_reader("STORAGE");
		if (!reader.has_value())
		{
			LOG_ERROR("CReader creation error");
		}
		storage::CNonVolatileStorage::ReadBinaryResult readResult = reader.value().read_binary("BinaryData");
		if (!readResult.data.has_value())
		{
			LOG_ERROR("Error reading data using CReader");
		}
		for (uint8_t byte : readResult.data.value()) 
		{
        	std::printf("%02X ", byte);
    	}
		std::printf("\n");
		
		storage::CNonVolatileStorage::ReadBinaryResult readResult2 = writer.value().read_binary("BinaryData");
		if (!readResult2.data.has_value())
		{
			LOG_ERROR("Error reading data using CReadWriter");
		}
		for (uint8_t byte : readResult2.data.value()) 
		{
        	std::printf("%02X ", byte);
    	}
		std::printf("\n");

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
