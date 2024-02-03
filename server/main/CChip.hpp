#pragma once

#include <cstdint>
#include <string>


namespace chip
{
// free func because this value makes no sense to store. And might want to call it mid app without instantiating the entire chip info.
[[nodiscard]] uint32_t min_free_heap();

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
	uint8_t m_Cores;
	uint16_t m_Revision;
	uint16_t m_Model;
	uint32_t m_Features;
};
}	// namespace chip