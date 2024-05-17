#include "CNonVolatileStorage.hpp"
#include "../server_common.hpp"
#include "../ble/ble_common.hpp"
// third_party
#include "nvs_handle.hpp"
//
//
//
//
namespace
{
void throw_read_write_constructor_error(storage::NvsErrorCode error, std::string_view nameSpace)
{
    using namespace storage;
    ASSERT(error != NvsErrorCode::notInitilized, "Trying to open nvs when storage driver is not initialized");
    ASSERT(error != NvsErrorCode::partitionNotFound, "Partition with specified name is not found in the partition table");
    ASSERT(error != NvsErrorCode::noSpaceForNewEntry, "No space for a new entry or there are too many different namespaces");

    if (error == NvsErrorCode::fail)
    {
        throw std::runtime_error("Internal error when trying to open NVS. Most likely due to corrupted NVS partition!");
    }
    else if (error == NvsErrorCode::namespaceNotFound)
    {
        throw std::runtime_error(FMT("Namespace with id {} doesn't exist", nameSpace.data()));
    }
    else if (error == NvsErrorCode::invalidName)
    {
        throw std::runtime_error(FMT("{} is an invalid namespace name", nameSpace.data()));
    }
    else
    {
        LOG_FATAL("Unknown error when trying to open a handle to NVS");
    }
}
void log_write_error(storage::NvsErrorCode& result)
{
    using namespace storage;
    static constexpr std::string_view msg = "Failed to write to NVS. Reason: ";

    if (result == NvsErrorCode::fail)
    {
        LOG_ERROR_FMT("{}{}", msg.data(), "Internal error; most likely due to corrupted NVS partition");
    }
    else if (result == NvsErrorCode::invalidHandle)
    {
        LOG_ERROR_FMT("{}{}", msg.data(), "Handle has been closed or is NULL");
    }
    else if (result == NvsErrorCode::readOnly)
    {
        LOG_ERROR_FMT("{}{}", msg.data(), "Handle was opened as read only");
    }
    else if (result == NvsErrorCode::invalidName)
    {
        LOG_ERROR_FMT("{}{}", msg.data(), "Key name doesn't satisfy constraints");
    }
    else if (result == NvsErrorCode::noSpaceForNewEntry)
    {
        LOG_ERROR_FMT("{}{}", msg.data(), "There is not enough space to save the value");
    }
    else if (result == NvsErrorCode::failedWriteOperation)
    {
        LOG_ERROR_FMT("{}{}", msg.data(), "write operation has failed. The value was written however.");
    }
    else if (result == NvsErrorCode::dataToLarge)
    {
        LOG_ERROR_FMT("{}{}", msg.data(), "The given data is to large");
    }
    else
    {
        LOG_ERROR_FMT("{}{}", msg.data(), "Unknown error occurred");
        result = NvsErrorCode::unknown;
    }
}
}    // namespace
namespace storage
{
CNonVolatileStorage::CHandle::CHandle(OpenMode mode, std::string_view nameSpace)
    : m_Handle{ UINT32_MAX }
{
    esp_err_t result = nvs_open(nameSpace.data(), static_cast<nvs_open_mode_t>(mode), &m_Handle);
    ASSERT(ble::EspErrorCode{ result } != ble::EspErrorCode::noMemory, "Memory could not be allocated for the internal structure");
    if (NvsErrorCode{ result } != NvsErrorCode::success)
    {
        throw_read_write_constructor_error(NvsErrorCode{ result }, nameSpace);
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
    : m_Handle{ std::exchange(other.m_Handle, UINT32_MAX) }
{}
CNonVolatileStorage::CHandle& CNonVolatileStorage::CHandle::operator=(CHandle&& other) noexcept
{
    if (this != &other)
    {
        m_Handle = std::exchange(other.m_Handle, UINT32_MAX);
    }
    return *this;
}
std::optional<CNonVolatileStorage::CHandle> CNonVolatileStorage::CHandle::make_handle(OpenMode mode, std::string_view nameSpace)
{
    [[maybe_unused]] const CNonVolatileStorage& nvs = CNonVolatileStorage::instance();
    try
    {
        return CHandle{ mode, nameSpace };
        // cannot use return std::make_optional<CHandle>(mode, nameSpace);
        // problaby because the constructor is private, but why does it work for CReader and CReadWriter?
    }
    catch (const exception::fatal_error& err)
    {
        throw;
    }
    catch (const std::runtime_error& err)
    {
        LOG_ERROR_FMT("Failed to create NVS handle. Reason: \"{}\"", err.what());
        return std::nullopt;
    }
}
nvs_handle_t& CNonVolatileStorage::CHandle::handle()
{
    return m_Handle;
}
CNonVolatileStorage::CReader::CReader(std::string_view nameSpace)
    : m_Handle{ CHandle::make_handle(OpenMode::readOnly, nameSpace) }
{
    if (!m_Handle.has_value())
    {
        throw std::runtime_error("Error trying to open NVS. Invalid m_Handle value!");
    }
}
std::optional<storage::CNonVolatileStorage::CReader> CNonVolatileStorage::CReader::make_reader(std::string_view nameSpace)
{
    try
    {
        return std::make_optional<CReader>(CReader{ nameSpace });
    }
    catch (const std::runtime_error& e)
    {
        // TODO: handle error here
        return std::nullopt;
    }
}
std::expected<std::vector<uint8_t>, NvsErrorCode> CNonVolatileStorage::CReader::read_binary(std::string_view key)
{
    ASSERT(key.size() < (NVS_KEY_NAME_MAX_SIZE - 1), "The given Key was to large");
    size_t requiredSize{};
    NvsErrorCode result = static_cast<NvsErrorCode>(nvs_get_blob(m_Handle.value().handle(), key.data(), nullptr, &requiredSize));
    if (result == NvsErrorCode::success)
    {
        std::expected<std::vector<uint8_t>, NvsErrorCode> expectedData{};
        expectedData->resize(requiredSize);

        result = static_cast<NvsErrorCode>(nvs_get_blob(m_Handle.value().handle(), key.data(), expectedData->data(), &requiredSize));
        if (result == NvsErrorCode::success)
        {
            return expectedData;
        }
    }

    return std::unexpected{ result };
}
std::expected<int8_t, NvsErrorCode> CNonVolatileStorage::CReader::read_int8(std::string_view key)
{
    ASSERT(key.size() < (NVS_KEY_NAME_MAX_SIZE - 1), "The given Key was to large");

    std::expected<int8_t, NvsErrorCode> expectedValue{ 0 };
    NvsErrorCode result = static_cast<NvsErrorCode>(nvs_get_i8(m_Handle.value().handle(), key.data(), &(*expectedValue)));
    if (result != NvsErrorCode::success)
    {
        return std::unexpected{ result };
    }

    return expectedValue;
};
CNonVolatileStorage::CWriter::CWriter(std::string_view nameSpace)
    : m_Handle{ CHandle::make_handle(OpenMode::readAndWrite, nameSpace) }
{
    if (!m_Handle.has_value())
    {
        throw std::runtime_error("Error trying to open NVS. Invalid m_Handle value!");
    }
}    // CReader constructor
std::optional<storage::CNonVolatileStorage::CWriter> CNonVolatileStorage::CWriter::make_writer(std::string_view nameSpace)
{
    try
    {
        return std::make_optional<CWriter>(nameSpace);
    }
    catch (const std::runtime_error& e)
    {
        // TODO: handle error here
        return std::nullopt;
    }
}
NvsErrorCode CNonVolatileStorage::CWriter::write_binary(std::string_view key, const std::vector<uint8_t>& data)
{
    ASSERT(key.size() < (NVS_KEY_NAME_MAX_SIZE - 1), "The given Key was to large");
    NvsErrorCode result = static_cast<NvsErrorCode>(nvs_set_blob(m_Handle.value().handle(), key.data(), data.data(), data.size()));
    if (result != NvsErrorCode::success)
    {
        log_write_error(result);
        return result;
    }

    return commit();
}
NvsErrorCode CNonVolatileStorage::CWriter::write_int8(std::string_view key, int8_t data)
{
    ASSERT(key.size() < (NVS_KEY_NAME_MAX_SIZE - 1), "The given Key was to large");
    NvsErrorCode result = static_cast<NvsErrorCode>(nvs_set_i8(m_Handle.value().handle(), key.data(), data));
    if (result != NvsErrorCode::success)
    {
        log_write_error(result);
        return result;
    }

    return commit();
}
NvsErrorCode CNonVolatileStorage::CWriter::commit()
{
    // return std::optional<Error>
    NvsErrorCode result = static_cast<NvsErrorCode>(nvs_commit(m_Handle.value().handle()));
    if (result != NvsErrorCode::success)
    {
        if (result == NvsErrorCode::invalidHandle)
        {
            LOG_ERROR("Failed to write to NVS. Handle has been closed or is NULL");
        }
        else
        {
            LOG_ERROR("Failed to write to NVS. Error from the underlying storage driver");
            result = NvsErrorCode::unknown;
        }
    }

    return result;
}
CNonVolatileStorage::CNonVolatileStorage()
{
    esp_err_t initResult{};
    do
    {
        initResult = nvs_flash_init();
        if (!success(initResult))
        {
            if (initResult == ESP_ERR_NVS_NO_FREE_PAGES)
            {
                LOG_WARN_FMT("Initialize NVS default partition failed with {}.. erasing and trying again..", initResult);
                auto eraseResult = nvs_flash_erase();
                if (!success(eraseResult))
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
// void CNonVolatileStorage::erase_all_key_value_pairs()
//{
//	// esp_err_t nvs_erase_all(nvs_handle_t handle)
// }
}    // namespace storage
