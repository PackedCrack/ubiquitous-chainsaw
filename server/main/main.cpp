#include "sys/CSystem.hpp"
#include "ble/CNimble.hpp"

#include "sys/CNonVolatileStorage.hpp"

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
	std::printf("\nCurrent minimum free heap: %lu bytes\n\n", system.min_free_heap());
}




extern "C" void app_main(void)
{
	sys::CSystem system{};
	try 
	{
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


		//std::printf("Total entries: %d\n", stats.total_entries); // This represents the total number of key-value pairs that can be stored in the NVS partition
		//std::printf("Used entries: %d\n", stats.used_entries); //: This indicates the number of key-value pairs that are currently stored in the NVS partition.
		//std::printf("Free entries: %d\n", stats.free_entries); // This represents the number of available slots for storing additional key-value pairs in the NVS partition. 
		//std::printf("Namespace entries: %d\n", stats.namespace_count);


		// esp_err_t nvs_erase_key(nvs_handle_t handle, const char *key)
		// esp_err_t nvs_flash_erase(void) // Erase the default NVS partition.
		// esp_err_t nvs_erase_all(nvs_handle_t handle) // Erase all key-value pairs in a namespace.
		// esp_err_t nvs_get_stats(const char *part_name, nvs_stats_t *nvs_stats)
		// esp_err_t nvs_get_used_entry_count(nvs_handle_t handle, size_t *used_entries)
		// esp_err_t nvs_get_stats(const char *part_name, nvs_stats_t *nvs_stats)


		// https://mynewt.apache.org/latest/
		//{
        //ble::CNimble tmp {};
		//}
		//ble::CNimble nimble {};
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
		system.restart();
    }
}
