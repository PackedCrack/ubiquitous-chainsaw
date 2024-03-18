#pragma once
// std
#include <optional>
// third_party
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"


namespace sys
{
class CEventGroup
{
public:
	CEventGroup();
	~CEventGroup();
	CEventGroup(const CEventGroup& other) = delete;	// TODO:: implemnt
	CEventGroup(CEventGroup&& other) = delete;
	CEventGroup& operator=(const CEventGroup& other) = delete;
	CEventGroup& operator=(CEventGroup&& other) = delete;

	void set_bit(const EventBits_t bits);
	[[nodiscard]] EventBits_t wait_for_any_bit(const EventBits_t bits, TickType_t waitTime = portMAX_DELAY, BaseType_t clearOnReturn = pdFALSE) const;
	[[nodiscard]] EventBits_t wait_for_all_bits(const EventBits_t bits, TickType_t waitTime = portMAX_DELAY, BaseType_t clearOnReturn = pdFALSE) const;
	[[nodiscard]] std::optional<const EventGroupHandle_t> handle() const;
private:
	EventGroupHandle_t m_Handle;
};
}	// namespace system