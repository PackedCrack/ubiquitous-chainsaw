#include "CNonVolatileStorage.hpp"
#include "../server_common.hpp"
#include "../ble/ble_common.hpp"
// third_party
#include "nvs_handle.hpp"

namespace
{
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

storage::CNonVolatileStorage::WriteResult handle_write_error(storage::NvsErrorCode result)
{
	using namespace storage;
	using nvs = CNonVolatileStorage;

	if (result == NvsErrorCode::fail)
		return  nvs::WriteResult { .code = NvsErrorCode::fail, .msg = "Internal error; most likely due to corrupted NVS partition" };
	else if (result == NvsErrorCode::invalidHandle)
		return nvs::WriteResult { .code = NvsErrorCode::invalidHandle, .msg = "Handle has been closed or is NULL" };
	else if (result == NvsErrorCode::readOnly)
		return  nvs::WriteResult { .code = NvsErrorCode::readOnly, .msg = "Handle was opened as read only" };
	else if (result == NvsErrorCode::invalidName)
		return  nvs::WriteResult { .code = NvsErrorCode::invalidName, .msg = "Key name doesn't satisfy constraints" };
	else if (result == NvsErrorCode::noSpaceForNewEntry)
		return  nvs::WriteResult { .code = NvsErrorCode::noSpaceForNewEntry, .msg = "There is not enough space to save the value" };
	else if (result == NvsErrorCode::failedWriteOperation)
		return  nvs::WriteResult { .code = NvsErrorCode::failedWriteOperation, .msg = "write operation has failed. The value was written however." };
	else if (result == NvsErrorCode::dataToLarge)
		return  nvs::WriteResult { .code = NvsErrorCode::dataToLarge, .msg = "The given data is to large" };
	else
		return  nvs::WriteResult { .code = NvsErrorCode::unknown, .msg = "Unknown error occured" };
}


} // namespace
namespace storage
{
CNonVolatileStorage::CHandle::CHandle(OpenMode mode, std::string_view nameSpace)
	: m_Handle { UINT32_MAX }	
{
	esp_err_t result = nvs_open(nameSpace.data(), static_cast<nvs_open_mode_t>(mode), &m_Handle);
	if (m_Handle == UINT32_MAX)
	{
		throw std::invalid_argument("Error trying to open NVS. Invalid m_Handle value!");
	}
	if (result != static_cast<esp_err_t>(NvsErrorCode::success))
	{
		handle_read_write_constructor_error(result, nameSpace);
	}
}
CNonVolatileStorage::CHandle::~CHandle()
{
	if (m_Handle != UINT32_MAX)
	{
		return;
	}
	nvs_close(m_Handle);
	m_Handle = UINT32_MAX;
}
CNonVolatileStorage::CHandle::CHandle(CHandle&& other) noexcept
	: m_Handle { std::exchange(other.m_Handle, UINT32_MAX) }
{}
CNonVolatileStorage::CHandle& CNonVolatileStorage::CHandle::operator=(CHandle&& other) noexcept
{
	if(this != &other)
	{
		m_Handle = std::exchange(other.m_Handle, UINT32_MAX);
	}
	return *this;
}
std::optional<CNonVolatileStorage::CHandle> CNonVolatileStorage::CHandle::make_handle(OpenMode mode, std::string_view nameSpace)
{
	[[maybe_unused]] CNonVolatileStorage& nvs = CNonVolatileStorage::instance();
	try
	{
		return CHandle {mode, nameSpace};
		// cannot use return std::make_optional<CHandle>(mode, nameSpace);
		// problaby because the constructor is private, but why does it work for CReader and CReadWriter?
	}
	catch(const std::invalid_argument& e)
	{
		// TODO: handle error here
		return std::nullopt;
	}
}
nvs_handle_t& CNonVolatileStorage::CHandle::handle()
{
	return m_Handle;
}

CNonVolatileStorage::CReader::CReader(std::string_view nameSpace)
	: m_Handle { CHandle::make_handle(OpenMode::readOnly, nameSpace) }
{
	if (!m_Handle.has_value())
	{
		throw std::invalid_argument("Error trying to open NVS. Invalid m_Handle value!");
	}
} // CReader constructor
CNonVolatileStorage::CReader::CReader(CReader&& other) noexcept
	: m_Handle { std::exchange(other.m_Handle, std::nullopt) }
{}
CNonVolatileStorage::CReader& CNonVolatileStorage::CReader::operator=(CReader&& other) noexcept
{
	if(this != &other)
	{;
		m_Handle = std::exchange(other.m_Handle, std::nullopt);
	}
	return *this;
}
std::optional<storage::CNonVolatileStorage::CReader> CNonVolatileStorage::CReader::make_reader(std::string_view nameSpace)
{
	[[maybe_unused]] CNonVolatileStorage& nvs = CNonVolatileStorage::instance();
	try
	{
		return std::make_optional<CReader>( nameSpace );
	}
	catch(const std::invalid_argument& e)
	{
		// TODO: handle error here
		return std::nullopt;
	}
}

CNonVolatileStorage::ReadBinaryResult CNonVolatileStorage::CReader::read_binary(std::string_view key)
{

	ASSERT(key.size() < (NVS_KEY_NAME_MAX_SIZE - 1), "The given Key was to large");
	size_t requiredSize {};
	NvsErrorCode result = static_cast<NvsErrorCode>(nvs_get_blob(m_Handle.value().handle(), key.data(), nullptr, &requiredSize));
	if (result == NvsErrorCode::success)
	{
		std::vector<uint8_t> retrievedData {};
		retrievedData.resize(requiredSize); 
		result = static_cast<NvsErrorCode>(nvs_get_blob(m_Handle.value().handle(), key.data(), retrievedData.data(), &requiredSize));
		if (result == NvsErrorCode::success)
		{
			return ReadBinaryResult { .code = NvsErrorCode::success, 
									  .data = std::make_optional<std::vector<uint8_t>>( std::move(retrievedData) )};
		}
		else 
		{

			if (result == NvsErrorCode::fail)
			{
				return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::fail, .data = std::nullopt };
			}
			else if (result == NvsErrorCode::namespaceNotFound)
			{
				return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::namespaceNotFound, .data = std::nullopt };
			}
			else if (result == NvsErrorCode::invalidHandle)
			{
				return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::invalidHandle, .data = std::nullopt };
			}
			else if (result == NvsErrorCode::invalidName)
			{
				return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::invalidName, .data = std::nullopt };
			}
			else if (result == NvsErrorCode::invalidDataLenght)
			{
				return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::invalidDataLenght, .data = std::nullopt };
			}
			else
			{
				LOG_FATAL("CReader::read_binary(): Unknown error occured!");
				return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::unknown, .data = std::nullopt };
			}
		}
	}
	else
	{
		if (result == NvsErrorCode::fail)
		{
			return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::fail, .data = std::nullopt };
		}
		else if (result == NvsErrorCode::namespaceNotFound)
		{
			return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::namespaceNotFound, .data = std::nullopt };
		}
		else if (result == NvsErrorCode::invalidHandle)
		{
			return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::invalidHandle, .data = std::nullopt };
		}
		else if (result == NvsErrorCode::invalidName)
		{
			return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::invalidName, .data = std::nullopt };
		}
		else if (result == NvsErrorCode::invalidDataLenght)
		{
			return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::invalidDataLenght, .data = std::nullopt };
		}
		else
		{
			LOG_FATAL("CReader::read_binary(): Unknown error occured!");
			return CNonVolatileStorage::ReadBinaryResult { .code = NvsErrorCode::unknown, .data = std::nullopt };
		}
	} 
}


CNonVolatileStorage::Readint8Result CNonVolatileStorage::CReader::read_int8(std::string_view key)
{
	ASSERT(key.size() < (NVS_KEY_NAME_MAX_SIZE - 1), "The given Key was to large");
	
	int8_t value = 0;
	NvsErrorCode result = static_cast<NvsErrorCode>(nvs_get_i8(m_Handle.value().handle(), key.data(), &value));
	if (result == NvsErrorCode::success)
	{
		return Readint8Result { .code = NvsErrorCode::success, 
								  .data = std::make_optional<int8_t>( value )};
	}
	else 
	{
		if (result == NvsErrorCode::fail)
		{
			return CNonVolatileStorage::Readint8Result { .code = NvsErrorCode::fail, .data = std::nullopt };
		}
		else if (result == NvsErrorCode::namespaceNotFound)
		{
			return CNonVolatileStorage::Readint8Result { .code = NvsErrorCode::namespaceNotFound, .data = std::nullopt };
		}
		else if (result == NvsErrorCode::invalidHandle)
		{
			return CNonVolatileStorage::Readint8Result { .code = NvsErrorCode::invalidHandle, .data = std::nullopt };
		}
		else if (result == NvsErrorCode::invalidName)
		{
			return CNonVolatileStorage::Readint8Result { .code = NvsErrorCode::invalidName, .data = std::nullopt };
		}
		else if (result == NvsErrorCode::invalidDataLenght)
		{
			return CNonVolatileStorage::Readint8Result { .code = NvsErrorCode::invalidDataLenght, .data = std::nullopt };
		}
		else
		{
			LOG_FATAL("CReader::read_binary(): Unknown error occured!");
			return CNonVolatileStorage::Readint8Result { .code = NvsErrorCode::unknown, .data = std::nullopt };
		}
	}
}

CNonVolatileStorage::CWriter::CWriter(std::string_view nameSpace)
	: m_Handle { CHandle::make_handle(OpenMode::readAndWrite, nameSpace) }
{
	if (!m_Handle.has_value())
	{
		throw std::invalid_argument("Error trying to open NVS. Invalid m_Handle value!");
	}
} // CReader constructor

CNonVolatileStorage::CWriter::CWriter(CWriter&& other) noexcept
	: m_Handle { std::exchange(other.m_Handle, std::nullopt) }
{}
CNonVolatileStorage::CWriter& CNonVolatileStorage::CWriter::operator=(CWriter&& other) noexcept
{
	if(this != &other)
	{
		m_Handle = std::exchange(other.m_Handle, std::nullopt);
	}
	return *this;
}
CNonVolatileStorage::WriteResult CNonVolatileStorage::CWriter::write_binary(std::string_view key, const std::vector<uint8_t>& data)
{
	ASSERT(key.size() < (NVS_KEY_NAME_MAX_SIZE - 1), "The given Key was to large");
	NvsErrorCode result = static_cast<NvsErrorCode>(nvs_set_blob(m_Handle.value().handle(), key.data(), data.data(), data.size()));
	if (result == NvsErrorCode::success)
	{
		return commit();
	}
	else
	{
		return handle_write_error(result);
	}
}

CNonVolatileStorage::WriteResult CNonVolatileStorage::CWriter::write_int8(std::string_view key, int8_t data)
{
	ASSERT(key.size() < (NVS_KEY_NAME_MAX_SIZE - 1), "The given Key was to large");
	NvsErrorCode result = static_cast<NvsErrorCode>(nvs_set_i8(m_Handle.value().handle(), key.data(), data));
	if (result == NvsErrorCode::success)
	{
		return commit();
	}
	else
	{
		return handle_write_error(result);
	}
}


std::optional<storage::CNonVolatileStorage::CWriter> CNonVolatileStorage::CWriter::make_writer(std::string_view nameSpace)
{
	[[maybe_unused]] CNonVolatileStorage& nvs = CNonVolatileStorage::instance();
	try
	{
		return std::make_optional<CWriter>( nameSpace );
	}
	catch(const std::invalid_argument& e)
	{
		// TODO: handle error here
		return std::nullopt;
	}
}
CNonVolatileStorage::WriteResult CNonVolatileStorage::CWriter::commit()
{
	// return std::optional<Error>
	NvsErrorCode result = static_cast<NvsErrorCode>(nvs_commit(m_Handle.value().handle()));
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
	static CNonVolatileStorage nvs{};
	return nvs;
}
std::optional<CNonVolatileStorage::CReader> CNonVolatileStorage::make_reader(std::string_view nameSpace)
{
	return CReader::make_reader(nameSpace);
}
std::optional<CNonVolatileStorage::CWriter> CNonVolatileStorage::make_writer(std::string_view nameSpace)
{
	return CWriter::make_writer(nameSpace);
}


//void CNonVolatileStorage::erase_all_key_value_pairs()
//{
//	// esp_err_t nvs_erase_all(nvs_handle_t handle)
//}
} // namespace storage
