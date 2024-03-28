#include "CNonVolatileStorage.hpp"
#include "../server_common.hpp"
#include "../ble/ble_common.hpp"
#include "defines.hpp"
// std
#include <stdexcept>
#include <cassert>
// third_party
#include "nvs_flash.h"
#include "nvs_handle.hpp"

namespace
{

storage::CNonVolatileStorage::ReadBinaryResult handle_nvs_read_error(storage::NvsErrorCode error)
{
	using namespace storage;

	if (error == NvsErrorCode::fail)
		return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::fail, .data = std::nullopt };
	else if (error == NvsErrorCode::namespaceNotFound)
		return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::namespaceNotFound, .data = std::nullopt };
	else if (error == NvsErrorCode::invalidHandle)
		return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::invalidHandle, .data = std::nullopt };
	else if (error == NvsErrorCode::invalidName)
		return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::invalidName, .data = std::nullopt };
	else if (error == NvsErrorCode::invalidDataLenght)
		return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::invalidDataLenght, .data = std::nullopt };
	else
	{
		LOG_FATAL("CReader::read_binary(): Unknown error occured!");
		return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::unknown, .data = std::nullopt };
	}
}
void handle_read_write_constructor_error(esp_err_t error, std::string_view nameSpace)
{
	using namespace storage;
	if (error == static_cast<esp_err_t>(NvsErrorCode::fail))
	{
		throw std::invalid_argument("Internal error when trying to open NVS. Most likely due to corrupted NVS partition!");
	}
	else if (error == static_cast<esp_err_t>(NvsErrorCode::notInitilized))
	{
		LOG_FATAL("CReader: Trying to open nvs when storage driver is not initilzied");
	}
	else if (error == static_cast<esp_err_t>(NvsErrorCode::partitionNotFound))
	{
		LOG_FATAL("CReader: Partition with specified name is not found in the partition table");
	}
	else if (error == static_cast<esp_err_t>(NvsErrorCode::namespaceNotFound))
	{
		std::string msg = FMT("Namespace with id {} doesen't exist", nameSpace.data());
		throw std::invalid_argument(msg);
	}
	else if (error == static_cast<esp_err_t>(NvsErrorCode::invalidName))
	{
		std::string msg = FMT("{} is an invalid namespace name", nameSpace.data());
		throw std::invalid_argument(msg);
	}
	else if (error == static_cast<esp_err_t>(ble::EspErrorCode::noMemory))
	{
		LOG_FATAL("CReader: Memory could not be allocated for the internal structure");
	}
	else if (error == static_cast<esp_err_t>(NvsErrorCode::noSpaceForNewEntry))
	{
		LOG_FATAL("CReader: No space for a new entry or there are too many different namespaces"); // handle by clearing unused namespaces?
	}
	else
	{
		LOG_FATAL("CReader: Unknown error");
	}
}

} // namespace
namespace storage
{
CNonVolatileStorage::CReader::CReader(std::string_view nameSpace)
	: m_Handle { UINT32_MAX }
{
	esp_err_t result = nvs_open(nameSpace.data(), static_cast<nvs_open_mode_t >(OpenMode::readOnly), &m_Handle.value());
	if (m_Handle.value() == UINT32_MAX)
	{
		throw std::invalid_argument("Error trying to open NVS. Invalid m_Handle value!");
	}

	if (result != static_cast<esp_err_t>(NvsErrorCode::success))
	{
		handle_read_write_constructor_error(result, nameSpace);
	}
} // CReader constructor
CNonVolatileStorage::CReader::~CReader()
{	
	if (m_Handle.has_value())
	{
		if (m_Handle.value() != UINT32_MAX)
		{
			return;
		}
		else
		{
			nvs_close(m_Handle.value());
			m_Handle = std::nullopt;
		}
	}
}
CNonVolatileStorage::CReader::CReader(CReader&& other) noexcept
	: m_Handle { std::exchange(other.m_Handle, std::nullopt) }
{}
CNonVolatileStorage::CReader& CNonVolatileStorage::CReader::operator=(CReader&& other) noexcept
{
	if(this != &other)
	{
		m_Handle = std::exchange(other.m_Handle, std::nullopt);
	}
	return *this;
}
CNonVolatileStorage::ReadBinaryResult CNonVolatileStorage::CReader::read_binary(std::string_view key)
{
	ASSERT(key.size() < (NVS_KEY_NAME_MAX_SIZE - 1), "The given Key was to large");

	size_t requiredSize {};
	esp_err_t result = nvs_get_blob(m_Handle.value(), key.data(), nullptr, &requiredSize);

	NvsErrorCode error = static_cast<NvsErrorCode>(result);
	if (error == NvsErrorCode::success)
	{
		std::vector<uint8_t> retrievedData {};
		retrievedData.resize(requiredSize); // make sure this is correct size
		result = nvs_get_blob(m_Handle.value(), key.data(), retrievedData.data(), &requiredSize);

		error = static_cast<NvsErrorCode>(result);
		if (error == NvsErrorCode::success)
		{
			return ReadBinaryResult { .code = NvsErrorCode::success, .data = std::move(retrievedData)};
		}
		else 
		{
			return handle_nvs_read_error(error);
		}
	}
	else
	{
		return handle_nvs_read_error(error);
	} 
}
CNonVolatileStorage::CReadWriter::CReadWriter(std::string_view nameSpace)
	: m_Handle { UINT32_MAX }
{
	esp_err_t result = nvs_open(nameSpace.data(), static_cast<nvs_open_mode_t >(OpenMode::readAndWrite), &m_Handle.value());
	if (m_Handle.value() == UINT32_MAX)
	{
		throw std::invalid_argument("Error trying to open NVS. Invalid m_Handle value!");
	}

	if (result != static_cast<esp_err_t>(NvsErrorCode::success))
	{
		handle_read_write_constructor_error(result, nameSpace);
	}
} // CReader constructor
CNonVolatileStorage::CReadWriter::~CReadWriter()
{
	if (m_Handle.has_value())
	{
		if (m_Handle.value() != UINT32_MAX)
		{
			return;
		}
		else
		{
			nvs_close(m_Handle.value());
			m_Handle = std::nullopt;
		}
	}
}
CNonVolatileStorage::CReadWriter::CReadWriter(CReadWriter&& other) noexcept
	: m_Handle { std::exchange(other.m_Handle, std::nullopt) }
{}
CNonVolatileStorage::CReadWriter& CNonVolatileStorage::CReadWriter::operator=(CReadWriter&& other) noexcept
{
	if(this != &other)
	{
		m_Handle = std::exchange(other.m_Handle, std::nullopt);
	}
	return *this;
}
CNonVolatileStorage::WriteResult CNonVolatileStorage::CReadWriter::write_binary(std::string_view key, const std::vector<uint8_t>& data)
{
	ASSERT(key.size() < (NVS_KEY_NAME_MAX_SIZE - 1), "The given Key was to large");

	NvsErrorCode result = static_cast<NvsErrorCode>(nvs_set_blob(m_Handle.value(), key.data(), data.data(), (data.size() * sizeof(uint8_t))));
	if (result == NvsErrorCode::success)
		return commit();
	else if (result == NvsErrorCode::fail)
		return WriteResult { .code = NvsErrorCode::fail, .msg = "Internal error; most likely due to corrupted NVS partition" };
	else if (result == NvsErrorCode::invalidHandle)
		return WriteResult { .code = NvsErrorCode::invalidHandle, .msg = "Handle has been closed or is NULL" };
	else if (result == NvsErrorCode::readOnly)
		return WriteResult { .code = NvsErrorCode::readOnly, .msg = "Handle was opened as read only" };
	else if (result == NvsErrorCode::invalidName)
		return WriteResult { .code = NvsErrorCode::invalidName, .msg = "Key name doesn't satisfy constraints" };
	else if (result == NvsErrorCode::noSpaceForNewEntry)
		return WriteResult { .code = NvsErrorCode::noSpaceForNewEntry, .msg = "There is not enough space to save the value" };
	else if (result == NvsErrorCode::failedWriteOperation)
		return WriteResult { .code = NvsErrorCode::failedWriteOperation, .msg = "write operation has failed. The value was written however." };
	else if (result == NvsErrorCode::dataToLarge)
		return WriteResult { .code = NvsErrorCode::dataToLarge, .msg = "The given data is to large" };
	else
		return WriteResult { .code = NvsErrorCode::unknown, .msg = "Unknown error occured" };
}
CNonVolatileStorage::ReadBinaryResult CNonVolatileStorage::CReadWriter::read_binary(std::string_view key)
{
	ASSERT(key.size() < (NVS_KEY_NAME_MAX_SIZE - 1), "The given Key was to large");

	size_t requiredSize {};
	esp_err_t result = nvs_get_blob(m_Handle.value(), key.data(), nullptr, &requiredSize);

	NvsErrorCode error = static_cast<NvsErrorCode>(result);
	if (error == NvsErrorCode::success)
	{
		std::vector<uint8_t> retrievedData {};
		retrievedData.resize(requiredSize); // make sure this is correct size
		result = nvs_get_blob(m_Handle.value(), key.data(), retrievedData.data(), &requiredSize);

		error = static_cast<NvsErrorCode>(result);
		if (error == NvsErrorCode::success)
		{
			return ReadBinaryResult { .code = NvsErrorCode::success, .data = std::move(retrievedData)};
		}
		else 
		{
			return handle_nvs_read_error(error);
		}
	}
	else
	{
		return handle_nvs_read_error(error);
	} 
}
CNonVolatileStorage::WriteResult CNonVolatileStorage::CReadWriter::commit()
{
	NvsErrorCode result = static_cast<NvsErrorCode>(nvs_commit(m_Handle.value()));
	if (result != NvsErrorCode::success)
	{
		if (result == NvsErrorCode::invalidHandle)
		{
			return WriteResult {
				.code = NvsErrorCode::invalidHandle,
				.msg = "Handle has been closed or is NULL"
			};
		}
		else 
		{
			return WriteResult {
				.code = NvsErrorCode::unknown,
				.msg = "Error from the underlying storage driver"
			};
		}
	}
	return WriteResult {
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
		return std::make_optional<CReader>( nameSpace );

		//return CReader {nameSpace.data()};
	}
	catch(const std::invalid_argument& e)
	{
		// TODO: handle error here
		return std::nullopt;
	}
}
std::optional<CNonVolatileStorage::CReadWriter> CNonVolatileStorage::make_read_writer(std::string_view nameSpace)
{
	try
	{
		return std::make_optional<CReadWriter>( nameSpace );
	}
	catch(const std::invalid_argument& e)
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


}	// namespace storage

namespace storage
{
}