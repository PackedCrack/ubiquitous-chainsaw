#pragma once
// std
#include <cstdint>
#include <string>


namespace sys
{
class CChip
{
public:
    CChip();
    ~CChip() = default;
    CChip(const CChip& other) = default;
    CChip(CChip&& other) = default;
    CChip& operator=(const CChip& other) = default;
    CChip& operator=(CChip&& other) = default;

    [[nodiscard]] bool embedded_flash_memory();
    [[nodiscard]] bool wifi();
    [[nodiscard]] bool bluetooth_le();
    [[nodiscard]] bool bluetooth_classic();
    [[nodiscard]] bool IEEE_802_15_4();
    [[nodiscard]] bool embedded_psram();
    [[nodiscard]] std::string revision();
    [[nodiscard]] uint8_t cores();
private:
    uint32_t m_Features;
    uint16_t m_Revision;
    uint16_t m_Model;
    uint8_t m_Cores;
};
}   // namespace sys