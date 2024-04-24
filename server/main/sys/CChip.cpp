#include "CChip.hpp"
// third_party
#include "esp_chip_info.h"
namespace sys
{
CChip::CChip()
    : m_Features{ UINT32_MAX }
    , m_Revision{ UINT16_MAX }
    , m_Model{ UINT16_MAX }
    , m_Cores{ UINT8_MAX }
{
    esp_chip_info_t chipInfo{};
    esp_chip_info(&chipInfo);
    m_Cores = chipInfo.cores;
    m_Revision = chipInfo.revision;
    m_Model = chipInfo.model;
    m_Features = chipInfo.features;
}
bool CChip::embedded_flash_memory()
{
    return (m_Features & CHIP_FEATURE_EMB_FLASH) ? true : false;
}
bool CChip::wifi()
{
    return (m_Features & CHIP_FEATURE_WIFI_BGN) ? true : false;
}
bool CChip::bluetooth_le()
{
    return (m_Features & CHIP_FEATURE_BLE) ? true : false;
}
bool CChip::bluetooth_classic()
{
    return (m_Features & CHIP_FEATURE_BT) ? true : false;
}
bool CChip::IEEE_802_15_4()
{
    return (m_Features & CHIP_FEATURE_IEEE802154) ? true : false;
}
bool CChip::embedded_psram()
{
    return (m_Features & CHIP_FEATURE_EMB_PSRAM) ? true : false;
}
std::string CChip::revision()
{
    uint32_t major = m_Revision / 100u;
    uint32_t minor = m_Revision % 100u;
    return std::format("v{}.{}", major, minor);
    // return std::string{ "v" + std::to_string(major) + "." + std::to_string(minor) };
}
uint8_t CChip::cores()
{
    return m_Cores;
}
}    // namespace sys
