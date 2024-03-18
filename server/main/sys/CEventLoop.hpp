#pragma once

// std
#include <unordered_map>
// third_party
#include "esp_event.h"


namespace sys
{
class CEventHandlerInstance
{
	typedef esp_event_handler_t PFN_EventHandler;
public:
	CEventHandlerInstance(esp_event_base_t eventBase, int32_t eventID, PFN_EventHandler eventCallback, void* callbackArgs = nullptr);
	~CEventHandlerInstance();
	CEventHandlerInstance(const CEventHandlerInstance& other) = delete;	// TODO:: IMPLEMENT
	CEventHandlerInstance(CEventHandlerInstance&& other) noexcept;
	CEventHandlerInstance& operator=(const CEventHandlerInstance& other) = delete;
	CEventHandlerInstance& operator=(CEventHandlerInstance&& other) noexcept;

	[[nodiscard]] esp_event_base_t event_base() const;
private:
	esp_event_handler_instance_t m_Handle;
	esp_event_base_t m_EventBase;
	int32_t m_EventID;
};

class CEventLoop
{
public:
	CEventLoop();
	~CEventLoop();
	CEventLoop(const CEventLoop& other) = delete;	// TODO:: IMPLEMENT
	CEventLoop(CEventLoop&& other) = delete;
	CEventLoop& operator=(const CEventLoop& other) = delete;
	CEventLoop& operator=(CEventLoop&& other) = delete;

	void new_event_handler(CEventHandlerInstance&& eventHandlerInstance);
	void delete_event_handler(esp_event_base_t eventBase);

	[[nodiscard]] bool has_event(esp_event_base_t eventBase) const;
private:
	std::unordered_map<esp_event_base_t, CEventHandlerInstance> m_EventHandlerInstances;
};
}	// namespace system