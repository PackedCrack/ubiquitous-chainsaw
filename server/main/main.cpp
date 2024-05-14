#define WOLFSSL_ESPIDF
#define WOLFSSL_ESPWROOM32
#include <wolfssl/wolfcrypt/settings.h>

#include "sys/CSystem.hpp"
#include "ble/CNimble.hpp"
#include "sys/CNonVolatileStorage.hpp"

#include "security/CHash.hpp"
#include "security/sha.hpp"
#include "security/CWolfCrypt.hpp"
#include "security/CRandom.hpp"
#include "security/ecc_key.hpp"

#include "esp_system.h"
#include "server_common.hpp"
//
//
//
//
namespace
{
void init_singletons()
{
    const auto crypto = security::CWolfCrypt::instance();
    if (!crypto)
    {
        LOG_INFO("Tried to initilize WolfCrypt, but it has already been initlized!");
    }
    [[maybe_unused]] const storage::CNonVolatileStorage& nvs = storage::CNonVolatileStorage::instance();
}
}    // namespace
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
void measure_important_function(void)
{
    const unsigned MEASUREMENTS = 5'000;
    uint64_t start = esp_timer_get_time();

    using namespace storage;
    [[maybe_unused]] const CNonVolatileStorage& nvs = CNonVolatileStorage::instance();
    [[maybe_unused]] std::optional<CNonVolatileStorage::CReader> reader = CNonVolatileStorage::CReader::make_reader("STORAGE");

    for (int retries = 0; retries < MEASUREMENTS; retries++)
    {
        // CNonVolatileStorage::ReadBinaryResult result = reader.value().read_binary("BinaryData");
    }

    uint64_t end = esp_timer_get_time();

    printf("%u iterations took %llu milliseconds (%llu microseconds per invocation)\n",
           MEASUREMENTS,
           (end - start) / 1'000,
           (end - start) / MEASUREMENTS);
}
void write_key_to_nvs(std::string_view nameSpace, std::string_view key, const std::vector<uint8_t>& keyData)
{
    std::optional<storage::CNonVolatileStorage::CWriter> writer = storage::CNonVolatileStorage::CWriter::make_writer(nameSpace);
    if (!writer.has_value())
    {
        LOG_FATAL("Failed to initilize NVS CWriter");
    }

    auto writerResult = writer.value().write_binary(key, keyData);
    if (writerResult.code != storage::NvsErrorCode::success)
    {
        LOG_FATAL("FAILED TO WRITE ENC KEY");
    }
}
storage::CNonVolatileStorage::ReadResult<std::vector<uint8_t>> read_key_from_nvs(std::string_view nameSpace, std::string_view key)
{
    std::optional<storage::CNonVolatileStorage::CReader> reader = storage::CNonVolatileStorage::CReader::make_reader(nameSpace);
    if (!reader.has_value())
    {
        LOG_FATAL("Failed to initilize NVS CReader");
    }
    return reader.value().read_binary(key);
}
[[nodiscard]] auto make_load_key_invokable(std::string_view nameSpace, std::string_view key)
{
    return [nameSpace, key]() -> std::expected<std::vector<security::byte>, std::string>
    {
        auto readResult = read_key_from_nvs(nameSpace, key);
        if (readResult.code != storage::NvsErrorCode::success)
        {
            LOG_FATAL("Unable to read servers private encryption key");
        }
        return std::expected<std::vector<security::byte>, std::string>{ std::move(readResult.data.value()) };
    };
}
// void verify_ecc_keys()
//{
//	[[maybe_unused]] std::optional<security::CEccPublicKey> pubKey =
// security::make_ecc_key<security::CEccPublicKey>(make_load_key_invokable(NVS_ENC_NAMESPACE, NVS_ENC_PUB_KEY));
//     [[maybe_unused]] std::optional<security::CEccPrivateKey> privKey =
//     security::make_ecc_key<security::CEccPrivateKey>(make_load_key_invokable(NVS_ENC_NAMESPACE, NVS_ENC_PRIV_KEY));
//
//	security::CRandom rng = security::CRandom::make_rng().value();
//	std::string_view msg = "Very nice message";
//	security::CHash<security::Sha2_256> hash{ msg };
//	std::vector<security::byte> signature = privKey.value().sign_hash(rng, hash);
//	bool verified = pubKey.value().verify_hash(signature, hash);
//     if (verified)
//	{
//         std::printf("\nSignature Verified Successfully.\n");
//     }
//	else
//	{
//         std::printf("\nFailed to verify Signature.\n");
//     }
// }

// std::vector<uint8_t> SERVER_PRIV{
//     0x30, 0x31, 0x02, 0x01, 0x01, 0x04, 0x20, 0x22, 0x38, 0x5c, 0x68, 0xe1, 0x12, 0x60, 0x1a, 0x03,
//     0x14, 0xb3, 0x33, 0xf3, 0xbd, 0x84, 0x23, 0xcb, 0x29, 0x75, 0xcf, 0x1c, 0x8b, 0xf7, 0x57, 0xb6,
//     0xc9, 0x1b, 0xbf, 0x62, 0xfc, 0x04, 0xa9, 0xa0, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
//     0x03, 0x01, 0x07
// };
// std::vector<uint8_t> server_public{
//     0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a,
//     0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x17, 0x15, 0xaf, 0x5e, 0xbb,
//     0xb6, 0xd6, 0x76, 0xe3, 0x6b, 0xcc, 0xe5, 0x30, 0xb2, 0xaf, 0xb8, 0xc6, 0x38, 0x09, 0x7e, 0xc1,
//     0x8f, 0x29, 0xae, 0x6f, 0x14, 0x9e, 0xfb, 0x38, 0xe8, 0x09, 0x34, 0x8f, 0xc2, 0xed, 0x87, 0xe0,
//     0x6d, 0xff, 0x16, 0x0c, 0x5e, 0x4e, 0x5d, 0x75, 0xcc, 0xfa, 0xab, 0x0a, 0x1f, 0xa9, 0x66, 0x22,
//     0x45, 0xb4, 0xd6, 0xe5, 0x7f, 0xc5, 0x31, 0xe0, 0xd3, 0xe3, 0xcd
// };
// std::vector<uint8_t> client_public{
//     0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a,
//     0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xa2, 0xa2, 0xf5, 0xc6, 0xf8,
//     0x95, 0x20, 0xe0, 0x74, 0x39, 0x83, 0x23, 0x0d, 0x00, 0xca, 0xb9, 0xfa, 0x61, 0xf8, 0x0c, 0x28,
//     0x64, 0xd5, 0x66, 0xd2, 0x67, 0xb8, 0xcb, 0x03, 0x34, 0x7d, 0x6e, 0x86, 0xe0, 0xc9, 0x4b, 0xa6,
//     0x1d, 0xe3, 0x70, 0xe7, 0xa2, 0x74, 0x0c, 0xcb, 0xd8, 0x75, 0x2d, 0x22, 0x93, 0xec, 0x8c, 0x7e,
//     0x29, 0x00, 0x72, 0xfc, 0x98, 0x42, 0x18, 0xb3, 0x69, 0x06, 0x22
// };

#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "sdkconfig.h"

static const char* TAG = "example";
static uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];
void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t* event)
{
    /* initialization */
    size_t rx_size = 0;

    /* read */
    esp_err_t ret = tinyusb_cdcacm_read(static_cast<tinyusb_cdcacm_itf_t>(itf), buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Data from channel %d:", itf);
        ESP_LOG_BUFFER_HEXDUMP(TAG, buf, rx_size, ESP_LOG_INFO);
    }
    else
    {
        ESP_LOGE(TAG, "Read error");
    }

    /* write back */
    tinyusb_cdcacm_write_queue(static_cast<tinyusb_cdcacm_itf_t>(itf), buf, rx_size);
    tinyusb_cdcacm_write_flush(static_cast<tinyusb_cdcacm_itf_t>(itf), 0);
}
void tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t* event)
{
    int dtr = event->line_state_changed_data.dtr;
    int rts = event->line_state_changed_data.rts;
    ESP_LOGI(TAG, "Line state changed on channel %d: DTR:%d, RTS:%d", itf, dtr, rts);
}
extern "C" void app_main(void)
{
    sys::CSystem system{};

    ESP_LOGI(TAG, "USB initialization");
    tinyusb_config_t config{};
    tusb_desc_device_t deviceDescriptor{ .bLength = sizeof(tusb_desc_device_t),
                                         .bDescriptorType = TUSB_DESC_DEVICE,
                                         .bcdUSB = 0x02'00,    // USB specification version - 2.0
                                         .bDeviceClass = TUSB_CLASS_CDC,
                                         .bDeviceSubClass = MISC_SUBCLASS_COMMON,
                                         .bDeviceProtocol = MISC_PROTOCOL_IAD,
                                         .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
                                         .idVendor = 0xDE'AD,
                                         .idProduct = 0x13'37,
                                         .bcdDevice = 0x01'00,     // Device's firmware version - 1.0
                                         .iManufacturer = 0x01,    // Index for the string containing the manufacturer name
                                         .iProduct = 0x02,         // Index for the string containing the product name
                                         .iSerialNumber = 0x03,    // Index for the string containing the serial number
                                         .bNumConfigurations = 1 };
    config.device_descriptor = &deviceDescriptor;
    // Indexing string descriptors start at 1. Index 0 is reserved for indicating that there are no string descriptors
    std::array<const char*, 4> stringDescriptors{ nullptr, "Manufacturer", "Chainsaw Access Token", "Serial Number" };
    config.string_descriptor = stringDescriptors.data();

    ESP_ERROR_CHECK(tinyusb_driver_install(&config));

    tinyusb_config_cdcacm_t acm_cfg = { .usb_dev = TINYUSB_USBDEV_0,
                                        .cdc_port = TINYUSB_CDC_ACM_0,
                                        .rx_unread_buf_sz = 64,
                                        .callback_rx = &tinyusb_cdc_rx_callback,    // the first way to register a callback
                                        .callback_rx_wanted_char = NULL,
                                        .callback_line_state_changed = NULL,
                                        .callback_line_coding_changed = NULL };

    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
    /* the second way to register a callback */
    ESP_ERROR_CHECK(
        tinyusb_cdcacm_register_callback(TINYUSB_CDC_ACM_0, CDC_EVENT_LINE_STATE_CHANGED, &tinyusb_cdc_line_state_changed_callback));

#if (CONFIG_TINYUSB_CDC_COUNT > 1)
    acm_cfg.cdc_port = TINYUSB_CDC_ACM_1;
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
    ESP_ERROR_CHECK(
        tinyusb_cdcacm_register_callback(TINYUSB_CDC_ACM_1, CDC_EVENT_LINE_STATE_CHANGED, &tinyusb_cdc_line_state_changed_callback));
#endif

    ESP_LOGI(TAG, "USB initialization DONE");


    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(10'000));    // milisecs
    }


    try
    {
        // print_chip_info();
        init_singletons();


        // verify_ecc_keys();
        // write_key_to_nvs(NVS_ENC_NAMESPACE, NVS_ENC_PRIV_KEY, SERVER_PRIV);
        // write_key_to_nvs(NVS_ENC_NAMESPACE, NVS_ENC_PUB_KEY, server_public);
        // write_key_to_nvs(NVS_ENC_NAMESPACE, NVS_ENC_CLIENT_KEY, client_public);


        using namespace storage;
        ble::CNimble nimble{};

        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1'000));    // milisecs
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
