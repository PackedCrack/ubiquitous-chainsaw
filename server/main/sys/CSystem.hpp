#pragma once
#include "../server_common.hpp"
#include "CChip.hpp"
// std
#include <array>
#include <type_traits>
// esp
#include "esp_system.h"
#include "esp_mac.h"
namespace sys
{
enum class ResetReason : int32_t
{
    unknown = ESP_RST_UNKNOWN,              //!< Reset reason can not be determined
    powerOn = ESP_RST_POWERON,              //!< Reset due to power-on event
    externPin = ESP_RST_EXT,                //!< Reset by external pin (not applicable for ESP32)
    softwareReset = ESP_RST_SW,             //!< Software reset via esp_restart
    panic = ESP_RST_PANIC,                  //!< Software reset due to exception/panic
    watchdogInterrupt = ESP_RST_INT_WDT,    //!< Reset (software or hardware) due to interrupt watchdog
    watchdogTask = ESP_RST_TASK_WDT,        //!< Reset due to task watchdog
    watchdogOther = ESP_RST_WDT,            //!< Reset due to other watchdogs
    deepSleep = ESP_RST_DEEPSLEEP,          //!< Reset after exiting deep sleep mode
    brownout = ESP_RST_BROWNOUT,            //!< Brownout reset (software or hardware)
    sdio = ESP_RST_SDIO                     //!< Reset over SDIO
};
enum class MacType : int32_t
{
    wifiStation = ESP_MAC_WIFI_STA,
    wifiSoftAP = ESP_MAC_WIFI_SOFTAP,
    bluetooth = ESP_MAC_BT,
    ethernet = ESP_MAC_ETH,
    IEEE802154 = ESP_MAC_IEEE802154,
    base = ESP_MAC_BASE,
    efuseFactory = ESP_MAC_EFUSE_FACTORY,
    efuseCustom = ESP_MAC_EFUSE_CUSTOM,
};
[[nodiscard]] constexpr std::string_view mac_type_to_str(MacType type)
{
    UNHANDLED_CASE_PROTECTION_ON
    switch (type)
    {
    case MacType::wifiStation:
        return "Wifi Station";
    case MacType::wifiSoftAP:
        return "Wifi Soft AP";
    case MacType::bluetooth:
        return "Bluetooth";
    case MacType::ethernet:
        return "Ethernet";
    case MacType::IEEE802154:
        return "IEEE802154";
    case MacType::base:
        return "Base";
    case MacType::efuseFactory:
        return "Efuse Factory";
    case MacType::efuseCustom:
        return "Efuse Custom";
    }
    UNHANDLED_CASE_PROTECTION_OFF

    __builtin_unreachable();
}
class CSystem
{
public:
    enum class MacError
    {
        none,
        macIsNull,
        macNotSet,
        invalidVersion,
        invalidCrc,
        unknown
    };
#if CONFIG_IEEE802154_ENABLED
    using AddressArray = std::array<uint8_t, 8u>;
#else
    using AddressArray = std::array<uint8_t, 6u>;
#endif
private:
    struct CustomMac
    {};
    struct DefaultMac
    {};
public:
    // TODO:
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv429esp_register_shutdown_handler18shutdown_handler_t
    // esp_err_t esp_register_shutdown_handler(shutdown_handler_t handle)
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv431esp_unregister_shutdown_handler18shutdown_handler_t
    // esp_err_t esp_unregister_shutdown_handler(shutdown_handler_t handle)

    void restart();
    [[nodiscard]] ResetReason reset_reason() const;
    [[nodiscard]] CChip chip_info() const;
    [[nodiscard]] uint32_t free_heap();
    [[nodiscard]] uint32_t free_internal_heap();
    [[nodiscard]] uint32_t min_free_heap();
    [[nodiscard]] size_t mac_address_size(MacType type) const;
    [[nodiscard]] MacError set_base_mac_address(const std::array<uint8_t, 6u>& address) const;
    [[nodiscard]] Result<std::array<uint8_t, 6u>, MacError> base_mac_address() const;
    [[nodiscard]] Result<AddressArray, MacError> custom_efuse_mac_address() const;
    [[nodiscard]] Result<AddressArray, MacError> default_efuse_mac_address() const;
    template<size_t size>
    [[nodiscard]] MacError set_interface_mac_address(MacType type, const std::array<uint8_t, size>& address)
    {
        if (MacType::IEEE802154 == type)
        {
            ASSERT(address.size() == 8u, "IEEE802154 Address requires 8 bytes!");
        }
        else
        {
            ASSERT(address.size() == 6u, "Non IEEE802154 address' should be 6 bytes!");
        }

        // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv422esp_iface_mac_addr_setPK7uint8_t14esp_mac_type_t
        esp_err_t result = esp_iface_mac_addr_set(address.data(), static_cast<esp_mac_type_t>(type));
        MacError err = to_mac_error(result);
        if (err == MacError::unknown)
        {
            LOG_ERROR_FMT("Failed with code: \"{}\" - \"{}\". When trying to set mac address to: \"{}\" for mac type: \"{}\".",
                          result,
                          esp_err_to_str(result),
                          address,
                          mac_type_to_str(type));
        }

        return err;
    }
    // TODO:
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv412esp_read_macP7uint8_t14esp_mac_type_t
    // esp_err_t esp_read_mac(uint8_t *mac, esp_mac_type_t type
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv420esp_derive_local_macP7uint8_tPK7uint8_t
    // esp_err_t esp_derive_local_mac(uint8_t *local_mac, const uint8_t *universal_mac)
private:
    [[nodiscard]] constexpr MacError to_mac_error(esp_err_t err) const;
};
}    // namespace sys
