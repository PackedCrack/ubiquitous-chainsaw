#include "CNonVolatileStorage.hpp"
#include "../server_common.hpp"
#include "../ble/ble_common.hpp"
// std
#include <stdexcept>
#include <cassert>
// third_party
#include "nvs_flash.h"
#include "nvs_handle.hpp"

namespace
{

} // namespace
namespace storage
{
CNonVolatileStorage::CReader::CReader(std::string_view nameSpace)
	: m_Handle {}
{
	// NOTE: can return 2 error types
    esp_err_t  result = nvs_open(nameSpace.data(), static_cast<nvs_open_mode_t >(OpenMode::readOnly), &m_Handle);
	//NvsErrorCode error = static_cast<NvsErrorCode>(result);
		//storage::CNonVolatileStorage::ReadBinaryResult tmp = read_binary("BinaryData");
		//if (!tmp.data.has_value())
		//{
		//	std::printf("Error %d\n", static_cast<esp_err_t>(tmp.code)); // invalidHandle ???
		//	LOG_ERROR("Error reading data");
		//}
		//
		//std::vector<uint8_t> data = tmp.data.value();
		//std::printf("Data size=%d\n", data.size());
		//for (uint8_t byte : data) 
		//{
        //	std::printf("%02X ", byte);
    	//}


	if (result != static_cast<esp_err_t>(NvsErrorCode::success))
	{
		if (result == static_cast<esp_err_t>(NvsErrorCode::fail))
		{
			throw std::runtime_error("Internal error when trying to open NVS in CReader constructor. Most likely due to corrupted NVS partition!");
		}
		else if (result == static_cast<esp_err_t>(NvsErrorCode::notInitilized))
		{
			LOG_FATAL("CReader: Trying to open nvs when storage driver is not initilzied");
		}
		else if (result == static_cast<esp_err_t>(NvsErrorCode::partitionNotFound))
		{
			LOG_FATAL("CReader: Partition with specified name is not found in the partition table");
		}
		else if (result == static_cast<esp_err_t>(NvsErrorCode::namespaceNotFound))
		{
			std::string msg = "Namespace with id '" + std::string(nameSpace) + "' doesen't exist";
			throw std::runtime_error(msg);
		}
		else if (result == static_cast<esp_err_t>(NvsErrorCode::invalidName))
		{
			std::string msg = "'" + std::string(nameSpace) + "' is an invalid namespace name";
			throw std::runtime_error(msg);
		}
		else if (result == static_cast<esp_err_t>(ble::EspErrorCode::noMemory))
		{
			LOG_FATAL("CReader: Memory could not be allocated for the internal structure");
		}
		else if (result == static_cast<esp_err_t>(NvsErrorCode::noSpaceForNewEntry))
		{
			LOG_FATAL("CReader: No space for a new entry or there are too many different namespaces"); // handle by clearing unused namespaces?
		}
		else
		{
			LOG_FATAL("CReader: Unknown error");
		}
	}
} // CReader constructor
CNonVolatileStorage::CReader::~CReader()
{
	nvs_close(m_Handle);
}
CNonVolatileStorage::ReadBinaryResult CNonVolatileStorage::CReader::read_binary(std::string_view key)
{
	// Key = "BinaryData"
	ASSERT(key.size() < (NVS_KEY_NAME_MAX_SIZE - 1), "The given Key was to large");

	// Read binary data from NVS
	size_t requiredSize {};
	esp_err_t result = nvs_get_blob(m_Handle, key.data(), nullptr, &requiredSize);

	NvsErrorCode error = static_cast<NvsErrorCode>(result);
	if (error == NvsErrorCode::success)
	{
		std::vector<uint8_t> retrievedData {};
		retrievedData.reserve(requiredSize);
		result = nvs_get_blob(m_Handle, key.data(), retrievedData.data(), &requiredSize);
		error = static_cast<NvsErrorCode>(result);
		if (error == NvsErrorCode::success)
		{
			return ReadBinaryResult { .data = retrievedData, .code = NvsErrorCode::success};
		}
		else 
		{
			if (error == NvsErrorCode::fail)
				return ReadBinaryResult { .data = std::nullopt, .code = NvsErrorCode::fail };
			else if (error == NvsErrorCode::namespaceNotFound)
				return ReadBinaryResult { .data = std::nullopt, .code = NvsErrorCode::namespaceNotFound };
			else if (error == NvsErrorCode::invalidHandle)
				return ReadBinaryResult { .data = std::nullopt, .code = NvsErrorCode::invalidHandle };
			else if (error == NvsErrorCode::invalidName)
				return ReadBinaryResult { .data = std::nullopt, .code = NvsErrorCode::invalidName };
			else if (error == NvsErrorCode::invalidDataLenght)
				return ReadBinaryResult { .data = std::nullopt, .code = NvsErrorCode::invalidDataLenght };
			else
			{
				LOG_FATAL("CReader::read_binary(): Unknown error occured!");
				return ReadBinaryResult { .data = std::nullopt, .code = NvsErrorCode::unknown }; // to disable control reaches end of non-void func
			}
		}
	}
	else
	{
		if (error == NvsErrorCode::fail)
			return ReadBinaryResult { .data = std::nullopt, .code = NvsErrorCode::fail };
		else if (error == NvsErrorCode::namespaceNotFound)
			return ReadBinaryResult { .data = std::nullopt, .code = NvsErrorCode::namespaceNotFound };
		else if (error == NvsErrorCode::invalidHandle)
			return ReadBinaryResult { .data = std::nullopt, .code = NvsErrorCode::invalidHandle };
		else if (error == NvsErrorCode::invalidName)
			return ReadBinaryResult { .data = std::nullopt, .code = NvsErrorCode::invalidName };
		else if (error == NvsErrorCode::invalidDataLenght)
			return ReadBinaryResult { .data = std::nullopt, .code = NvsErrorCode::invalidDataLenght };
		else
		{
			LOG_FATAL("CReader::read_binary(): Unknown error occured!");
			return ReadBinaryResult { .data = std::nullopt, .code = NvsErrorCode::unknown }; // to disable control reaches end of non-void func
		}
	} 
}
CNonVolatileStorage::CReadWriter::CReadWriter(std::string_view nameSpace)
	: m_Handle {}
{
	// NOTE: Can return 2 types of errors
    esp_err_t result = nvs_open(nameSpace.data(), static_cast<nvs_open_mode_t >(OpenMode::readAndWrite), &m_Handle);
	if (result != static_cast<esp_err_t>(NvsErrorCode::success))
	{
		if (result == static_cast<esp_err_t>(NvsErrorCode::fail))
		{
			throw std::runtime_error("Internal error when trying to open NVS in CReader constructor. Most likely due to corrupted NVS partition!");
		}
		else if (result == static_cast<esp_err_t>(NvsErrorCode::notInitilized))
		{
			LOG_FATAL("CReader: Trying to open nvs when storage driver is not initilzied");
		}
		else if (result == static_cast<esp_err_t>(NvsErrorCode::partitionNotFound))
		{
			LOG_FATAL("CReader: Partition with specified name is not found in the partition table");
		}
		else if (result == static_cast<esp_err_t>(NvsErrorCode::namespaceNotFound))
		{
			std::string msg = "Namespace with id '" + std::string(nameSpace) + "' doesen't exist";
			throw std::runtime_error(msg);
		}
		else if (result == static_cast<esp_err_t>(NvsErrorCode::invalidName))
		{
			std::string msg = "'" + std::string(nameSpace) + "' is an invalid namespace name";
			throw std::runtime_error(msg);
		}
		else if (result == static_cast<esp_err_t>(ble::EspErrorCode::noMemory))
		{
			LOG_FATAL("CReader: Memory could not be allocated for the internal structure");
		}
		else if (result == static_cast<esp_err_t>(NvsErrorCode::noSpaceForNewEntry))
		{
			LOG_FATAL("CReader: No space for a new entry or there are too many different namespaces"); // handle by clearing unused namespaces?
		}
		else
		{
			LOG_FATAL("CReader: Unknown error");
		}
	}
} // CReader constructor
CNonVolatileStorage::CReadWriter::~CReadWriter()
{
	nvs_close(m_Handle);
}
CNonVolatileStorage::Error CNonVolatileStorage::CReadWriter::write_binary(std::string_view key, const std::vector<uint8_t>& data)
{

// "BinaryData"
	// SAVE BINARY DATA
	//std::vector<uint8_t> binaryData = { 0xBA, 0xBE, 0xBA, 0xBe, 0xBA, 0xBE, 0xBA, 0xBE, 0xBA, 0xBE, 0xBA, 0xBE, 0xBA, 0xBE, 0xBA, 0xBE };

	// Write binary data to NVS
	NvsErrorCode result = static_cast<NvsErrorCode>(nvs_set_blob(m_Handle, key.data(), data.data(), (data.size() * sizeof(uint8_t))));
	if (result == NvsErrorCode::success)
		return commit();
	else if (result == NvsErrorCode::fail)
		return Error { .code = NvsErrorCode::fail, .msg = "Internal error; most likely due to corrupted NVS partition" };
	else if (result == NvsErrorCode::invalidHandle)
		return Error { .code = NvsErrorCode::invalidHandle, .msg = "Handle has been closed or is NULL" };
	else if (result == NvsErrorCode::readOnly)
		return Error { .code = NvsErrorCode::readOnly, .msg = "Handle was opened as read only" };
	else if (result == NvsErrorCode::invalidName)
		return Error { .code = NvsErrorCode::invalidName, .msg = "Key name doesn't satisfy constraints" };
	else if (result == NvsErrorCode::noSpaceForNewEntry)
		return Error { .code = NvsErrorCode::noSpaceForNewEntry, .msg = "There is not enough space to save the value" };
	else if (result == NvsErrorCode::failedWriteOperation)
		return Error { .code = NvsErrorCode::failedWriteOperation, .msg = "write operation has failed. The value was written however." };
	else if (result == NvsErrorCode::dataToLarge)
		return Error { .code = NvsErrorCode::dataToLarge, .msg = "The given data is to large" };
	else
		return Error { .code = NvsErrorCode::unknown, .msg = "Unknown error occured" };

	// TODO: Check if this is more optimal
	//size_t existingDataSize = 0;
	//esp_err_t result = nvs_get_blob(m_Handle, key.data(), NULL, &existingDataSize);
	//if (result != ESP_OK && result != ESP_ERR_NVS_NOT_FOUND) 
	//{
	//    nvs_close(m_Handle);
	//    return result;
	//}
	//if (existingDataSize > 0) 
	//{
	//	//// Allocate memory to store the existing blob
	//	//std::vector<uint8_t> existingBinaryData(existingSize);
	//	//err = nvs_get_blob(my_handle, "BinaryData", existingBinaryData.data(), &existingSize);
	//	//if (err != ESP_OK) 
	//	//{
	//	//	nvs_close(my_handle);
	//	//	return err;
	//	//}
//
	//	bool isEqual = vectors_is_equal(binaryData, existingBinaryData);
	//	if (isEqual)
	//	{
	//		std::printf("vectors are equal no need to add again!\n");
	//		nvs_close(my_handle);
	//		return err;
	//	}
	//}
}
CNonVolatileStorage::Error CNonVolatileStorage::CReadWriter::commit()
{
	NvsErrorCode result = static_cast<NvsErrorCode>(nvs_commit(m_Handle));
	if (result != NvsErrorCode::success)
	{
		if (result == NvsErrorCode::invalidHandle)
		{
			return Error {
				.code = NvsErrorCode::invalidHandle,
				.msg = "Handle has been closed or is NULL"
			};
		}
		else 
		{
			return Error {
				.code = NvsErrorCode::unknown,
				.msg = "Error from the underlying storage driver"
			};
		}
	}

	return Error {
			.code = NvsErrorCode::success,
			.msg = "Success"
		};
}
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
std::optional<CNonVolatileStorage::CReader> CNonVolatileStorage::make_reader(std::string_view nameSpace)
{
	try
	{
		return CReader {nameSpace.data()};
	}
	catch(const std::runtime_error& e)
	{
		// TODO: handle error here
		return std::nullopt;
	}
}
std::optional<CNonVolatileStorage::CReadWriter> CNonVolatileStorage::make_read_writer(std::string_view nameSpace)
{
	try
	{
		return CReadWriter {nameSpace.data()};
	}
	catch(const std::runtime_error& e)
	{
		// TODO: handle error here
		return std::nullopt;
	}
}

//std::optional<StatsError> CNonVolatileStorage::stats(std::string_view partition)
//{
//	nvs_stats_t stats {};
//	esp_err_t result = nvs_get_stats(NULL, &stats);
//		if (result != static_cast<esp_err_t>(NvsErrorCode::success)) 
//		{
//			
//			if (result == static_cast<esp_err_t>(NvsErrorCode::partitionNotFound))
//			{
//				return StatsError 
//				{
//					.stats = std::nullopt,
//					.msg =  FMT("Partition with name "{}" was not found", partition.data());
//				};
//			}
//			else if (result == static_cast<esp_err_t>(NvsErrorCode::partitionNotFound))
//			{
//				return StatsError 
//				{
//					.stats = std::nullopt,
//					.msg =  ""
//				};
//			}
//			else if (result == static_cast<esp_err_t>(NvsErrorCode::notInitilized))
//			{
//				return StatsError 
//				{
//					.stats = std::nullopt,
//					.msg =  ""
//				};
//
//			}
//			else if (result == static_cast<esp_err_t>(NvsErrorCode::invalidArg))
//			{
//				return StatsError 
//				{
//					.stats = std::nullopt,
//					.msg =  ""
//				};
//
//			}
//			else if (result == static_cast<esp_err_t>(NvsErrorCode::invalidState))
//			{
//				return StatsError 
//				{
//					.stats = std::nullopt,
//					.msg =  ""
//				};
//
//			}
//
//	
//		}
//	return stats;
//} 


//std::optional<Error> CNonVolatileStorage::open(std::string_view key, CNonVolatileStorage::OpenMode openMode)
//{
//
//	nvs_handle_t nvshandle {};
//	// Note, can return other error codes from underlying storage driver, therefore i dont static_cast immediatly
//	esp_err_t result = nvs_open(key.data(), static_cast<nvs_open_mode_t>(OpenMode::readAndWrite), &nvshandle);
//
//	if (result != static_cast<esp_err_t>(NvsErrorCode::success)) 
//		return result;
//
//	/*
//	
//	ESP_OK if storage handle was opened successfully - 
//	ESP_FAIL if there is an internal error; most likely due to corrupted NVS partition (only if NVS assertion checks are disabled) -
//	 ESP_ERR_NVS_NOT_INITIALIZED if the storage driver is not initialized - 
//	 ESP_ERR_NVS_PART_NOT_FOUND if the partition with label "nvs" is not found - 
//	 ESP_ERR_NVS_NOT_FOUND id namespace doesn't exist yet and mode is NVS_READONLY - 
//	 ESP_ERR_NVS_INVALID_NAME if namespace name doesn't satisfy constraints - 
//	 ESP_ERR_NO_MEM in case memory could not be allocated for the internal structures - 
//	 ESP_ERR_NVS_NOT_ENOUGH_SPACE if there is no space for a new entry or there are too many different namespaces (maximum allowed different namespaces: 254) - other error codes from the underlying storage driver
//	*/
//
//	// return optional, either a handle or error code
//}
//
//void CNonVolatileStorage::close()
//{
//
//}
//
//void CNonVolatileStorage::commit()
//{
//
//}




}	// namespace storage
