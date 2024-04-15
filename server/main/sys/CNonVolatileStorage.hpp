#pragma once

/* NVS */
#include "nvs_flash.h"
/* Project*/
#include "../server_defines.hpp"
//#include "common/defines.hpp"
/* STD*/
#include <vector>
#include <string>
#include <string_view>
#include <optional>
namespace storage
{
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html#introduction
enum class NvsErrorCode : int32_t
{
	success = ESP_OK,
	fail = ESP_FAIL,
	notInitilized = ESP_ERR_NVS_NOT_INITIALIZED, // /*!< The storage driver is not initialized */
	namespaceNotFound = ESP_ERR_NVS_NOT_FOUND, /*!< A requested entry couldn't be found or namespace doesn’t exist yet and mode is NVS_READONLY */
	typeMismatch = ESP_ERR_NVS_TYPE_MISMATCH, /*!< The type of set or get operation doesn't match the type of value stored in NVS */
	readOnly = ESP_ERR_NVS_READ_ONLY, /*!< Storage handle was opened as read only */
	noSpaceForNewEntry = ESP_ERR_NVS_NOT_ENOUGH_SPACE, /*!< There is not enough space in the underlying storage to save the value */
	invalidName = ESP_ERR_NVS_INVALID_NAME, /*!< Namespace name doesn’t satisfy constraints */
	invalidHandle = ESP_ERR_NVS_INVALID_HANDLE, /*!< Handle has been closed or is NULL */
	failedWriteOperation = ESP_ERR_NVS_REMOVE_FAILED, /*!< The value wasn’t updated because flash write operation has failed. The value was written however, and update will be finished after re-initialization of nvs, provided that flash operation doesn’t fail again. */
	keyNameToLong = ESP_ERR_NVS_KEY_TOO_LONG, /*!< Key name is too long */
	invalidState = ESP_ERR_NVS_INVALID_STATE, /*!< NVS is in an inconsistent state due to a previous error. Call nvs_flash_init and nvs_open again, then retry. */
	invalidDataLenght = ESP_ERR_NVS_INVALID_LENGTH, /*!< String or blob length is not sufficient to store data */
	noFreePages = ESP_ERR_NVS_NO_FREE_PAGES, /*!< NVS partition doesn't contain any empty pages. This may happen if NVS partition was truncated. Erase the whole partition and call nvs_flash_init again. */
	dataToLarge = ESP_ERR_NVS_VALUE_TOO_LONG, /*!< Value doesn't fit into the entry or string or blob length is longer than supported by the implementation */
	partitionNotFound = ESP_ERR_NVS_PART_NOT_FOUND, /*!< Partition with specified name is not found in the partition table */
	unrecognizedDataFormat = ESP_ERR_NVS_NEW_VERSION_FOUND, /*!< NVS partition contains data in new format and cannot be recognized by this version of code */
	xtsEncryptWriteFail = ESP_ERR_NVS_XTS_ENCR_FAILED, /*!< XTS encryption failed while writing NVS entry */
	xtsDecryptReadFail = ESP_ERR_NVS_XTS_DECR_FAILED, /*!< XTS decryption failed while reading NVS entry */
	xtsConfigFail = ESP_ERR_NVS_XTS_CFG_FAILED, /*!< XTS configuration setting failed */
	xtsConfigNotFound = ESP_ERR_NVS_XTS_CFG_NOT_FOUND, /*!< XTS configuration not found */
	encryptionNotSupported = ESP_ERR_NVS_ENCR_NOT_SUPPORTED, /*!< NVS encryption is not supported in this version */
	keyPartNotInitilized = ESP_ERR_NVS_KEYS_NOT_INITIALIZED, /*!< NVS key partition is uninitialized */
	corruptedKey = ESP_ERR_NVS_CORRUPT_KEY_PART, /*!< NVS key partition is corrupt */
	wrongEncryption = ESP_ERR_NVS_WRONG_ENCRYPTION, /*!< NVS partition is marked as encrypted with generic flash encryption. This is forbidden since the NVS encryption works differently. */
	unknown = INT32_MAX
};
class CNonVolatileStorage
{
	enum class OpenMode
	{
		readOnly = NVS_READONLY,
		readAndWrite = NVS_READWRITE
	};
public:
	struct WriteResult
	{
		NvsErrorCode code;
		std::string msg; // make optional
	};
	struct ReadBinaryResult // make template ??
	{
		NvsErrorCode code;
		std::optional<std::vector<uint8_t>> data;
	};
	template<typename DerivedReader>
	class CBaseReader
	{
	public:
		[[nodiscard]] ReadBinaryResult read_binary(std::string_view key)
		{
			ASSERT(key.size() < (NVS_KEY_NAME_MAX_SIZE - 1), "The given Key was to large");
			size_t requiredSize {};
			NvsErrorCode result = static_cast<NvsErrorCode>(nvs_get_blob(static_cast<DerivedReader*>(this)->m_Handle.value(), 
																		key.data(), nullptr, &requiredSize));
			if (result == NvsErrorCode::success)
			{
				std::vector<uint8_t> retrievedData {};
				retrievedData.resize(requiredSize); 
				result = static_cast<NvsErrorCode>(nvs_get_blob(static_cast<DerivedReader*>(this)->m_Handle.value(), 
																key.data(), retrievedData.data(), &requiredSize));
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
		} // function bracket
	}; // class CBaseReader
	class CReader : public CBaseReader<CReader>
	{
	public:
		[[nodiscard]] static std::optional<storage::CNonVolatileStorage::CReader> make_reader(std::string_view nameSpace);
		~CReader();
		CReader(const CReader& other) = delete;
		CReader(CReader&& other) noexcept;
		CReader& operator=(const CReader& other) = delete;
		CReader& operator=(CReader&& other) noexcept;
	private:
	    explicit CReader(std::string_view nameSpace);
	private:
		std::optional<nvs_handle_t> m_Handle;
	}; // class CReader
	class CReadWriter : public CBaseReader<CReader>
	{
	public:
		[[nodiscard]] static std::optional<storage::CNonVolatileStorage::CReadWriter> make_read_writer(std::string_view nameSpace);
		~CReadWriter();
		CReadWriter(const CReadWriter& other) = delete;
		CReadWriter(CReadWriter&& other) noexcept;
		CReadWriter& operator=(const CReadWriter& other) = delete;
		CReadWriter& operator=(CReadWriter&& other) noexcept;
	private:
	    explicit CReadWriter(std::string_view nameSpace);
	public:
		[[nodiscard]] WriteResult write_binary(std::string_view key, const std::vector<uint8_t>& data);
	private:
		[[nodiscard]] WriteResult commit();
	private:
		std::optional<nvs_handle_t> m_Handle;
	}; // class CReadWriter
public:
	CNonVolatileStorage(); // MAKE PRIVATE
	~CNonVolatileStorage();
	CNonVolatileStorage(const CNonVolatileStorage& other) = delete;	// Deleted for now..
	CNonVolatileStorage(CNonVolatileStorage&& other) = delete;
	CNonVolatileStorage& operator=(const CNonVolatileStorage& other) = delete;
	CNonVolatileStorage& operator=(CNonVolatileStorage&& other) = delete;
public:
	[[nodiscard]] static CNonVolatileStorage& instance();
	[[nodiscard]] std::optional<CReader> make_reader(std::string_view nameSpace);
	[[nodiscard]] std::optional<CReadWriter> make_read_writer(std::string_view nameSpace);

	// esp_err_t nvs_erase_key(nvs_handle_t handle, const char *key)
	// esp_err_t nvs_flash_erase(void) // Erase the default NVS partition.
	// esp_err_t nvs_erase_all(nvs_handle_t handle) // Erase all key-value pairs in a namespace.
	// esp_err_t nvs_get_stats(const char *part_name, nvs_stats_t *nvs_stats)
	// esp_err_t nvs_get_used_entry_count(nvs_handle_t handle, size_t *used_entries)
}; // CNonVolatileStorage
}// namespace storage