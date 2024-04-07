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

// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/performance/speed.html#measuring-performance
void measure_important_function(void) {
    const unsigned MEASUREMENTS = 5000;
    uint64_t start = esp_timer_get_time();

	using namespace storage;
	[[maybe_unused]] CNonVolatileStorage& nvs = CNonVolatileStorage::instance();
	std::optional<CNonVolatileStorage::CReader> reader = CNonVolatileStorage::CReader::make_reader("STORAGE");

    for (int retries = 0; retries < MEASUREMENTS; retries++) 
	{
		//CNonVolatileStorage::ReadBinaryResult result = reader.value().read_binary("BinaryData");
    }

    uint64_t end = esp_timer_get_time();

    printf("%u iterations took %llu milliseconds (%llu microseconds per invocation)\n",
           MEASUREMENTS, (end - start)/1000, (end - start)/MEASUREMENTS);
}

extern "C" void app_main(void)
{
	sys::CSystem system{};
	try 
	{
		//std::expected<int, int> test{};
		print_chip_info();

		using namespace storage;

		[[maybe_unused]] CNonVolatileStorage& nvs = CNonVolatileStorage::instance();

		std::optional<CNonVolatileStorage::CReader> reader = CNonVolatileStorage::CReader::make_reader("STORAGE");
		CNonVolatileStorage::ReadBinaryResult readResult = reader.value().read_binary("BinaryData");
		if (readResult.code != NvsErrorCode::success)
		{
			LOG_ERROR("FAILED READ");
		}

		std::vector<uint8_t> data{0xFF, 0xFF, 0xFF};
		std::optional<CNonVolatileStorage::CWriter> writer = CNonVolatileStorage::CWriter::make_writer("STORAGE");
		CNonVolatileStorage::WriteResult writeResult = writer.value().write_binary("BinaryData", data);
		if (writeResult.code != NvsErrorCode::success)
		{
			LOG_ERROR("FAILED WRITE");
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
