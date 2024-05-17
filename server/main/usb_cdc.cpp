#include "usb_cdc.hpp"
#include "server_common.hpp"
#include "sys/CNonVolatileStorage.hpp"
// third-party
#include "esp_log.h"
//
//
//
//
namespace
{
static const char* TAG = "USB-CDC";
//static uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];
[[nodiscard]] std::string_view nvs_key(common::KeyType type)
{
    switch (type)
    {
    case common::KeyType::clientPublic:
        return NVS_KEY_CLIENT_PUBLIC;
    case common::KeyType::serverPublic:
        return NVS_KEY_SERVER_PUBLIC;
    case common::KeyType::serverPrivate:
        return NVS_KEY_SERVER_PRIVATE;
    }

    LOG_FATAL_FMT("Unexpected KeyType value given as parameter. Value: {}", std::to_underlying(type));
    std::unreachable();
}
[[nodiscard]] int8_t get_stored_key_count()
{
    using NVS = storage::CNonVolatileStorage;

    std::optional<NVS::CReader> reader = NVS::make_reader(NVS_ENCRYPTION_NAMESPACE);
    ASSERT(reader, "Failed to open reader to NVS");

    std::expected<int8_t, storage::NvsErrorCode> expectedResult = reader->read_int8(NVS_KEY_NUM_STORED_ENC_KEYS);
    ASSERT(expectedResult, "Failed to read value in NVS containing stored key count");
    return *expectedResult;
}
void update_stored_key_count()
{
    using NVS = storage::CNonVolatileStorage;

    int8_t count = get_stored_key_count() + 1;
    std::optional<NVS::CWriter> writer = NVS::make_writer(NVS_ENCRYPTION_NAMESPACE);
    ASSERT(writer, "Failed to open writer to NVS");
    ASSERT(writer->write_int8(NVS_KEY_NUM_STORED_ENC_KEYS, count) == storage::NvsErrorCode::success, "Failed to update key count in NVS.");
}
void store_key_in_nvs(common::KeyType keyType, const std::vector<uint8_t>& key)
{
    using NVS = storage::CNonVolatileStorage;

    std::optional<NVS::CWriter> writer = NVS::make_writer(NVS_ENCRYPTION_NAMESPACE);
    ASSERT(writer, "Failed to open writer to NVS");
    ASSERT(writer->write_binary(nvs_key(keyType), key) == storage::NvsErrorCode::success, "Failed to save key data to NVS.");
}
void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t* event)
{
    //static uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];
    static std::array<uint8_t, CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1> buf{};
    static auto key = std::make_pair<common::KeyType, std::vector<uint8_t>>(common::KeyType::undefined, {});
    /* initialization */
    size_t rx_size = 0;

    /* read */
    esp_err_t ret = tinyusb_cdcacm_read(static_cast<tinyusb_cdcacm_itf_t>(itf), buf.data(), CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
    if (ret == ESP_OK)
    {
        static std::size_t keySize{};
        if (key.first == common::KeyType::undefined || keySize == 0u)
        {
            LOG_INFO("Recieved and in first if statement");
            ASSERT(rx_size == 1, "Expected header to be sent in single byte increments");
            if (key.first == common::KeyType::undefined)
            {
                LOG_INFO("Changing key type");
                key.first = common::KeyType{ buf[0] };
            }
            else if (keySize == 0u)
            {
                keySize = buf[0];
                LOG_INFO_FMT("Changing key size to: \"{}\"", keySize);
                //key.second.reserve(keySize);
            }
        }
        else
        {
            LOG_INFO_FMT("Recieved \"{}\" and in else statement", rx_size);
            ASSERT(rx_size <= keySize, "Expected recieved data to be less than or equal to the key");
            for (std::size_t i = 0u; i < rx_size; ++i)
            {
                key.second.push_back(buf[i]);
                LOG_INFO_FMT("Key size after push_back: {}", key.second.size());
            }

            if (key.second.size() == keySize)
            {
//keys.emplace_back(std::move(key));
#ifdef NDEBUG
                ESP_LOG_BUFFER_HEXDUMP(TAG, keys.back().second.data(), keys.back().second.size(), ESP_LOG_INFO);
#endif

                store_key_in_nvs(key.first, key.second);
                update_stored_key_count();

                key = std::make_pair<common::KeyType, std::vector<uint8_t>>(common::KeyType::undefined, {});
                keySize = 0u;
                LOG_INFO("Emplaced back key to vector");
            }
            else if (key.second.size() > keySize)
            {
                LOG_ERROR("Size of Key vector is greater than keysize");
            }
        }

        ESP_LOGI(TAG, "Data from channel %d:", itf);
    }
    else
    {
        ESP_LOGE(TAG, "Read error");
    }

    /* write back */
    tinyusb_cdcacm_write_queue(static_cast<tinyusb_cdcacm_itf_t>(itf), buf.data(), rx_size);
    tinyusb_cdcacm_write_flush(static_cast<tinyusb_cdcacm_itf_t>(itf), 0);
}
void tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t* event)
{
    // TODO:
    [[maybe_unused]] int dtr = event->line_state_changed_data.dtr;
    [[maybe_unused]] int rts = event->line_state_changed_data.rts;
}
}    // namespace
namespace usb
{
// cppcheck-suppress constParameterReference
tinyusb_config_t make_config(const tusb_desc_device_t& deviceDescriptor, std::array<const char*, 3>& stringDescriptors)
{
    tinyusb_config_t config{};
    config.device_descriptor = &deviceDescriptor;
    config.string_descriptor =
        stringDescriptors.data();    // stringDescriptors cant be const& because tinyusb need 'const char**' not 'const char* const*'


    return config;
}
void init_usb(const tinyusb_config_t& config, const tinyusb_config_cdcacm_t& cdcConfig)
{
    ESP_ERROR_CHECK(tinyusb_driver_install(&config));
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&cdcConfig));

    ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(TINYUSB_CDC_ACM_0, CDC_EVENT_RX, &tinyusb_cdc_rx_callback));
    ESP_ERROR_CHECK(
        tinyusb_cdcacm_register_callback(TINYUSB_CDC_ACM_0, CDC_EVENT_LINE_STATE_CHANGED, &tinyusb_cdc_line_state_changed_callback));
}
}    // namespace usb
