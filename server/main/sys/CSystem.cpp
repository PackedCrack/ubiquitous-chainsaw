#include "CSystem.hpp"
#include "freertos/FreeRTOS.h"
namespace sys
{
CChip CSystem::chip_info() const
{
    return CChip{};
}
uint32_t CSystem::free_heap()
{
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv422esp_get_free_heap_sizev
    return esp_get_free_heap_size();
}
uint32_t CSystem::free_internal_heap()
{
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv431esp_get_free_internal_heap_sizev
    return esp_get_minimum_free_heap_size();
}
uint32_t CSystem::min_free_heap()
{
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv430esp_get_minimum_free_heap_sizev
    return esp_get_minimum_free_heap_size();
}
void CSystem::restart()
{
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv411esp_restartv
    esp_restart();
}
ResetReason CSystem::reset_reason() const
{
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv416esp_reset_reasonv
    return ResetReason{ esp_reset_reason() };
}
size_t CSystem::mac_address_size(MacType type) const
{
    return esp_mac_addr_len_get(static_cast<esp_mac_type_t>(type));
}
CSystem::MacError CSystem::set_base_mac_address(const std::array<uint8_t, 6u>& address) const
{
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv421esp_base_mac_addr_setPK7uint8_t
    return to_mac_error(esp_base_mac_addr_set(address.data()));
}
Result<std::array<uint8_t, 6u>, CSystem::MacError> CSystem::base_mac_address() const
{
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv421esp_base_mac_addr_getP7uint8_t
    std::array<uint8_t, 6u> addr{};
    esp_err_t result = esp_base_mac_addr_get(addr.data());

    return Result<std::array<uint8_t, 6u>, MacError>{ .value = success(result) ? std::make_optional<std::array<uint8_t, 6u>>(addr)
                                                                               : std::nullopt,
                                                      .error = to_mac_error(result) };
}
Result<CSystem::AddressArray, CSystem::MacError> CSystem::custom_efuse_mac_address() const
{
    // TODO
    AddressArray address{};
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv424esp_efuse_mac_get_customP7uint8_t
    esp_err_t result = esp_efuse_mac_get_custom(address.data());
    MacError err = to_mac_error(result);
    if (err == MacError::unknown)
    {
        LOG_ERROR_FMT("Failed with code: \"{}\" - \"{}\". When trying to retrieve the custom efuse mac address",
                      result,
                      esp_err_to_str(result));
    }

    return Result<AddressArray, MacError>{ .value = success(result) ? std::make_optional<AddressArray>(address) : std::nullopt,
                                           .error = err };
}
Result<CSystem::AddressArray, CSystem::MacError> CSystem::default_efuse_mac_address() const
{
    AddressArray address{};
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html#_CPPv425esp_efuse_mac_get_defaultP7uint8_t
    esp_err_t result = esp_efuse_mac_get_default(address.data());
    MacError err = to_mac_error(result);
    if (err == MacError::unknown)
    {
        LOG_ERROR_FMT("Failed with code: \"{}\" - \"{}\". When trying to retrieve the default efuse mac address",
                      result,
                      esp_err_to_str(result));
    }

    return Result<AddressArray, MacError>{ .value = success(result) ? std::make_optional<AddressArray>(address) : std::nullopt,
                                           .error = err };
}
constexpr CSystem::MacError CSystem::to_mac_error(esp_err_t err) const
{
    switch (err)
    {
    case ESP_OK:
        return MacError::none;
    case ESP_ERR_INVALID_ARG:
        return MacError::macIsNull;
    case ESP_ERR_INVALID_MAC:
        return MacError::macNotSet;
    case ESP_ERR_INVALID_VERSION:
        return MacError::invalidVersion;
    case ESP_ERR_INVALID_CRC:
        return MacError::invalidCrc;
    default:
        return MacError::unknown;
    }
}
}    // namespace sys
