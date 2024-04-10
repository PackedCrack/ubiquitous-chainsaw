#include "sys/CSystem.hpp"
#include "ble/CNimble.hpp"

#include "sys/CNonVolatileStorage.hpp"

#include "esp_system.h"

#include <expected>
#include "../../client/src/security/CWolfCrypt.hpp"
#include "server_common.hpp"


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

		//auto a = security::CWolfCrypt::instance();

		ble::CNimble nimble {};
		const ble::CGap* pGap = nimble.gap_handle();

		std::optional<CNonVolatileStorage::CWriter> rssiWriter = CNonVolatileStorage::CWriter::make_writer(NVS_RSSI_NAMESPACE);
		if (!rssiWriter.has_value())
		{
			LOG_FATAL("Failed to initilize NVS Writer");
		}

		std::optional<CNonVolatileStorage::CReader> rssiReader = CNonVolatileStorage::CReader::make_reader(NVS_RSSI_NAMESPACE);
		if (!rssiWriter.has_value())
		{
			LOG_FATAL("Failed to initilize NVS Writer");
		}

		CNonVolatileStorage::Readint8Result rssiReadResult = rssiReader.value().read_int8(NVS_RSSI_KEY);
		if (rssiReadResult.code == NvsErrorCode::success)
		{
			LOG_INFO_FMT("RSSI last saved rssi value = {}", rssiReadResult.data.value());
		}


		while (true)
		{
			// Perform any periodic tasks here
			std::optional<int8_t> rssiValue = pGap->rssi();
			if (rssiValue.has_value())
			{
				LOG_INFO_FMT("Rssi value = {}", rssiValue.value());
				CNonVolatileStorage::WriteResult rssiWriteResult = rssiWriter.value().write_int8(NVS_RSSI_KEY, rssiValue.value());
				if (rssiWriteResult.code != NvsErrorCode::success)
				{
					LOG_ERROR("FAILED RSSI WRITE");
				}

			}
			vTaskDelay(pdMS_TO_TICKS(1000)); // milisecs
		}
    } 
	catch (const exception::fatal_error& error) 
	{
		LOG_ERROR_FMT("Caught exception: {}", error.what());

		// TODO:: we should do more than restart here
		system.restart();
    }
}
