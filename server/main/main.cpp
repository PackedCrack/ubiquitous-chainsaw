#define WOLFSSL_ESPIDF
#define WOLFSSL_ESPWROOM32
#include <wolfssl/wolfcrypt/settings.h>

#include "usb_cdc.hpp"
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
#include "driver/gpio.h"
//
//
//
//
static QueueHandle_t gpio_evt_queue = nullptr;
namespace
{
void reset_stored_key_count([[maybe_unused]] void* arg)    // this arg is always nullptr as far as i can tell....
{
    while (true)
    {
        // real argument passed from isr is second parameter from this function
        if (xQueueReceive(gpio_evt_queue, nullptr, portMAX_DELAY))
        {
            using NVS = storage::CNonVolatileStorage;
            std::optional<NVS::CWriter> writer = NVS::make_writer(NVS_ENCRYPTION_NAMESPACE);
            ASSERT(writer, "Failed to open writer to NVS");
            ASSERT(writer->write_int8(NVS_KEY_NUM_STORED_ENC_KEYS, 0) == storage::NvsErrorCode::success,
                   "Failed to update key count in NVS.");

            sys::CSystem system{};
            system.restart();
        }
    }
}
void IRAM_ATTR button_isr_handler([[maybe_unused]] void* arg)
{
    xQueueSendFromISR(gpio_evt_queue, nullptr, nullptr);
}
void init_gpio()
{
    static constexpr gpio_config_t config{
        .pin_bit_mask = 1ULL << GPIO_NUM_5,    /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
        .mode = GPIO_MODE_INPUT,               /*!< GPIO mode: set input/output mode                     */
        .pull_up_en = GPIO_PULLUP_ENABLE,      /*!< GPIO pull-up                                         */
        .pull_down_en = GPIO_PULLDOWN_DISABLE, /*!< GPIO pull-down                                       */
        .intr_type = GPIO_INTR_NEGEDGE         /*!< GPIO interrupt type                                  */
    };
    ESP_ERROR_CHECK(gpio_config(&config));

    // xQueueGenericCreate( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, const uint8_t ucQueueType )
    gpio_evt_queue = xQueueCreate(10, 0);

    static constexpr uint32_t stackSize = 4'096u;
    xTaskCreate(reset_stored_key_count, "reset_stored_key_count", stackSize, nullptr, 10, nullptr);

    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_NUM_5, button_isr_handler, nullptr));
}
void create_nvs_encryption_namespace()
{
    using NVS = storage::CNonVolatileStorage;
    std::optional<NVS::CWriter> writer = NVS::make_writer(NVS_ENCRYPTION_NAMESPACE);
    if (!writer)
    {
        LOG_FATAL_FMT("Failed to count stored encryption keys. Could not open handle to NVS's namespace \"{}\".",
                      NVS_ENCRYPTION_NAMESPACE.data());
    }

    storage::NvsErrorCode result = writer->write_int8(NVS_KEY_NUM_STORED_ENC_KEYS, 0);
    ASSERT(result == storage::NvsErrorCode::success, "Expected success");
}
[[nodiscard]] int8_t num_stored_encryption_keys()
{
    using NVS = storage::CNonVolatileStorage;
    std::optional<NVS::CReader> reader = NVS::make_reader(NVS_ENCRYPTION_NAMESPACE);
    if (!reader)
    {
        LOG_FATAL_FMT("Failed to count stored encryption keys. Could not open handle to NVS's namespace \"{}\".",
                      NVS_ENCRYPTION_NAMESPACE.data());
    }

    std::expected<int8_t, storage::NvsErrorCode> expectedResult = reader->read_int8(NVS_KEY_NUM_STORED_ENC_KEYS);
    if (!expectedResult)
    {
        if (expectedResult.error() == storage::NvsErrorCode::namespaceNotFound)
        {
            create_nvs_encryption_namespace();
            return 0;
        }
        else
        {
            LOG_FATAL_FMT("Failed to count stored encryption keys. Could not read from NVS's namespace \"{}\". Reason: \"{}\"",
                          NVS_ENCRYPTION_NAMESPACE.data(),
                          std::to_underlying(expectedResult.error()));
        }
    }

    return *expectedResult;
}
void listen_for_serial_communication()
{
    static constexpr tinyusb_config_cdcacm_t cdcConfig = usb::make_cdc_config();
    // stringDescriptors cant be constexpr because tinyusb need 'const char**' not 'const char* const*'...
    std::array<const char*, 3> stringDescriptors{ "Manufacturer", "Chainsaw Access Token", "Serial Number" };
    tinyusb_config_t config = usb::make_config(usb::make_device_descriptor(), stringDescriptors);

    usb::init_usb(config, cdcConfig);

    while (num_stored_encryption_keys() < 3)
    {
        vTaskDelay(pdMS_TO_TICKS(1'000));    // milisecs
    }

    sys::CSystem system{};
    system.restart();
}
}    // namespace
extern "C" void app_main(void)
{
    sys::CSystem system{};

    try
    {
        {
            [[maybe_unused]] const storage::CNonVolatileStorage& nvs = storage::CNonVolatileStorage::instance();
            [[maybe_unused]] std::expected<security::CWolfCrypt*, security::CWolfCrypt::Error> wc = security::CWolfCrypt::instance();
            ASSERT(wc, "Tried to initilize WolfCrypt, but it has already been initlized!");
        }

        init_gpio();

        int8_t storedKeyCount = num_stored_encryption_keys();
        if (storedKeyCount == 3)
        {
            ble::CNimble nimble{};
            while (true)
            {
                vTaskDelay(pdMS_TO_TICKS(1'000));    // millisecs
            }
        }
        else
        {
            ASSERT(storedKeyCount == 0, "Expected stored key count to be 0 if its not 3.");
            listen_for_serial_communication();
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
