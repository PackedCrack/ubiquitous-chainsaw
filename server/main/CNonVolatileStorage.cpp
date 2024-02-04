#include "CNonVolatileStorage.hpp"
#include "defines.hpp"
// std
#include <stdexcept>
#include <cassert>
// third_party
#include "nvs_flash.h"
#include "nvs_handle.hpp"


namespace storage
{
	CNonVolatileStorage::CNonVolatileStorage()
	{
		esp_err_t initResult{};
		do
		{
			initResult = nvs_flash_init();
			if(!success(initResult))
			{
				if(initResult == ESP_ERR_NVS_NO_FREE_PAGES)
				{
					LOG_WARN_FMT("Initialize NVS default partition failed with {}.. erasing and trying again..", initResult);

					auto eraseResult = nvs_flash_erase();
					if(!success(eraseResult))
					{
						LOG_FATAL_FMT("ERASING DEFAULT FLASH PARTITION FAILED WITH {}", eraseResult);
					}
				}
				else
				{
					/* FROM DOCS
					one of the error codes from the underlying flash storage driver
					error codes from nvs_flash_read_security_cfg API (when “NVS_ENCRYPTION” is enabled).
					error codes from nvs_flash_generate_keys API (when “NVS_ENCRYPTION” is enabled).
					error codes from nvs_flash_secure_init_partition API (when “NVS_ENCRYPTION” is enabled) .
					*/
					LOG_FATAL_FMT("UNHANDLED FLASH INITIALIZATION RESULT: {}", initResult);
				}
			}
		} while (!success(initResult));

		LOG_INFO("Successfully initialized NVS default partition.");
	}

	CNonVolatileStorage::~CNonVolatileStorage()
	{
		auto result = nvs_flash_deinit();
		assert(result == ESP_OK);

	}

	CNonVolatileStorage& CNonVolatileStorage::instance()
	{
		LOG_INFO("BEFORE INIT NVS");
		static CNonVolatileStorage nvs{};
		LOG_INFO("AFTER INIT NVS");
		return nvs;
	}

}	// namespace storage