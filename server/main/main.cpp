#define WOLFSSL_ESPIDF
#define WOLFSSL_ESPWROOM32
//#define WOLFSSL_USER_SETTINGS
#include <wolfssl/wolfcrypt/settings.h>
//#include <wolfssl/version.h>
//#include <wolfssl/wolfcrypt/port/Espressif/esp32-crypt.h>

//#include <wolfssl/wolfcrypt/types.h>


#include "sys/CSystem.hpp"
#include "ble/CNimble.hpp"
#include "sys/CNonVolatileStorage.hpp"

//#define HAVE_ECC
#include "security/CHash.hpp"
#include "security/CWolfCrypt.hpp"
#include "security/CRandom.hpp"
#include "security/ecc_key.hpp"
#include "security/sha.hpp"

#include "esp_system.h"


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
	std::printf("\nCurrent minimum free heap: %u bytes\n\n", static_cast<unsigned int>(system.min_free_heap()));
}

// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/performance/speed.html#measuring-performance
void measure_important_function(void) {
    const unsigned MEASUREMENTS = 5000;
    uint64_t start = esp_timer_get_time();

	using namespace storage;
	[[maybe_unused]] const CNonVolatileStorage& nvs = CNonVolatileStorage::instance();
	[[maybe_unused]] std::optional<CNonVolatileStorage::CReader> reader = CNonVolatileStorage::CReader::make_reader("STORAGE");

    for (int retries = 0; retries < MEASUREMENTS; retries++) 
	{
		//CNonVolatileStorage::ReadBinaryResult result = reader.value().read_binary("BinaryData");
    }

    uint64_t end = esp_timer_get_time();

    printf("%u iterations took %llu milliseconds (%llu microseconds per invocation)\n",
           MEASUREMENTS, (end - start)/1000, (end - start)/MEASUREMENTS);
}


[[nodiscard]] auto make_load_key_invokable(std::string_view nameSpace, std::string_view key)
{
    return [nameSpace, key]() -> std::expected<std::vector<security::byte>, std::string>
    {
        std::expected<std::vector<security::byte>, std::string> data{};
		LOG_INFO_FMT("1: {}", nameSpace);
		LOG_INFO_FMT("2: {}", key);
        //static_assert(alignof(const char) == alignof(decltype(*(data->data()))));

        return data;
    
    };
}

extern "C" void app_main(void)
{
	sys::CSystem system{};
	try 
	{
		auto test = make_load_key_invokable("asv", "abc");

		//std::optional<security::CEccPublicKey> pubKey3 = security::make_ecc_key<security::CEccPublicKey>(make_load_invokable("PUBLIC_KEY"));
    	//std::optional<security::CEccPrivateKey> privKey3 = security::make_ecc_key<security::CEccPrivateKey>(make_load_invokable("PRIVATE_KEY"));
		 

		security::CRandom rng = security::CRandom::make_rng().value();
		security::CEccKeyPair keyPair {rng};
		auto pub = std::make_unique<security::CEccPublicKey>(keyPair.public_key());
		auto priv = std::make_unique<security::CEccPrivateKey>(keyPair.private_key());

		const char* msg = "Very nice message";
		security::CHash<security::Sha2_256> hash{ msg };
		std::vector<security::byte> signature = priv->sign_hash(rng, hash);
		bool verified = pub->verify_hash(signature, hash);
    	if (verified)
		{
    	    std::printf("\nSignature Verified Successfully.\n");
    	}
		else
		{
    	    std::printf("\nFailed to verify Signature.\n");
    	}
		
	

		//print_chip_info();

		using namespace storage;

		//[[maybe_unused]] const CNonVolatileStorage& nvs = CNonVolatileStorage::instance();
		
		//auto crypto = security::CWolfCrypt::instance();
		//if(crypto)
		//{
		//	std::printf("Works\n");
		//}


		//ble::CNimble nimble{};
		//std::expected<int, int> test{};
		print_chip_info();

		using namespace storage;

		[[maybe_unused]] const CNonVolatileStorage& nvs = CNonVolatileStorage::instance();

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
		
		auto rssiReadResult = rssiReader.value().read_int8(NVS_RSSI_KEY);
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
//
		// TODO:: we should do more than restart here
		system.restart();
    }
}
