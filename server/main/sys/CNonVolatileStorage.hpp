#pragma once

/* NVS */
#include "nvs_flash.h"
/* Project*/
#include "defines.hpp"
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

	struct Readint8Result // make template ??
	{
		NvsErrorCode code;
		std::optional<int8_t> data;
	};


	class CHandle 
	{
	private:
	    CHandle(OpenMode mode, std::string_view nameSpace);
	public:
		~CHandle();
		CHandle(const CHandle& other) = delete;
		CHandle(CHandle&& other) noexcept;
		CHandle& operator=(const CHandle& other) = delete;
		CHandle& operator=(CHandle&& other) noexcept;
	public:
		[[nodiscard]] static std::optional<CHandle> make_handle(OpenMode mode, std::string_view nameSpace);
		 [[nodiscard]] nvs_handle_t& handle();
	private:
		nvs_handle_t m_Handle;
	};
	class CReader
	{
	private:
	    CReader(std::string_view nameSpace);
	public:
		~CReader() = default;
		CReader(const CReader& other) = delete;
		CReader(CReader&& other) noexcept;
		CReader& operator=(const CReader& other) = delete;
		CReader& operator=(CReader&& other) noexcept;
	public:
		[[nodiscard]] static std::optional<storage::CNonVolatileStorage::CReader> make_reader(std::string_view nameSpace);
		[[nodiscard]] ReadBinaryResult read_binary(std::string_view key);
		[[nodiscard]] Readint8Result read_int8(std::string_view key);
	private:
		std::optional<CHandle> m_Handle;
	}; // class CReader
	class CWriter
	{
	private:
	    CWriter(std::string_view nameSpace);
	public:
		~CWriter() = default;
		CWriter(const CWriter& other) = delete;
		CWriter(CWriter&& other) noexcept;
		CWriter& operator=(const CWriter& other) = delete;
		CWriter& operator=(CWriter&& other) noexcept;
	public:
		[[nodiscard]] static std::optional<storage::CNonVolatileStorage::CWriter> make_writer(std::string_view nameSpace);
		[[nodiscard]] WriteResult write_binary(std::string_view key, const std::vector<uint8_t>& data);
		[[nodiscard]] WriteResult write_int8(std::string_view key, int8_t data);
	private:
		[[nodiscard]] WriteResult commit();
	private:
		std::optional<CHandle> m_Handle;
	}; // class CReadWriter

private:
	CNonVolatileStorage();
public:
	~CNonVolatileStorage();
	CNonVolatileStorage(const CNonVolatileStorage& other) = delete;	// Deleted for now..
	CNonVolatileStorage(CNonVolatileStorage&& other) = delete;
	CNonVolatileStorage& operator=(const CNonVolatileStorage& other) = delete;
	CNonVolatileStorage& operator=(CNonVolatileStorage&& other) = delete;
public:
	[[nodiscard]] static CNonVolatileStorage& instance();
	[[nodiscard]] std::optional<CReader> make_reader(std::string_view nameSpace);
	[[nodiscard]] std::optional<CWriter> make_writer(std::string_view nameSpace);

	// esp_err_t nvs_erase_key(nvs_handle_t handle, const char *key)
	// esp_err_t nvs_flash_erase(void) // Erase the default NVS partition.
	// esp_err_t nvs_erase_all(nvs_handle_t handle) // Erase all key-value pairs in a namespace.
	// esp_err_t nvs_get_stats(const char *part_name, nvs_stats_t *nvs_stats)
	// esp_err_t nvs_get_used_entry_count(nvs_handle_t handle, size_t *used_entries)
}; // CNonVolatileStorage
}// namespace storage