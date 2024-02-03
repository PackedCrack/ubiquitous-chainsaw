#include "CEventLoop.hpp"

#include "defines.hpp"
// std
#include <utility>
// third_party
#include "esp_err.h"


namespace sys
{
	//////////////////////////////////////////
	//		CEventHandlerInstance			//
	//////////////////////////////////////////
	CEventHandlerInstance::CEventHandlerInstance(esp_event_base_t eventBase, int32_t eventID, PFN_EventHandler eventCallback, void* callbackArgs)
		: m_Handle{ nullptr }
		, m_EventBase{ eventBase }
		, m_EventID{ eventID }
	{
		esp_err_t result = esp_event_handler_instance_register(m_EventBase, m_EventID, eventCallback, callbackArgs, &m_Handle);
		if(!success(result))
		{
			LOG_FATAL_FMT("FAILED WITH {}, WHEN TRYING TO REGISTER EVENT HANDLER INSTANCE", result);
		}
	}

	CEventHandlerInstance::~CEventHandlerInstance()
	{
		if(m_Handle == nullptr)	// Has been moved
			return;

		esp_err_t result = esp_event_handler_instance_unregister(m_EventBase, m_EventID, m_Handle);
		if(!success(result))
		{
			LOG_ERROR_ESP("FAILED WITH {}, WHEN TRYING TO UNREGISTER EVENT HANDLER INSTANCE", result);
		}
	}

	CEventHandlerInstance::CEventHandlerInstance(CEventHandlerInstance&& other) noexcept
		: m_Handle{ nullptr }
		, m_EventBase{ nullptr }
		, m_EventID{ std::move(other.m_EventID) }
	{
		std::swap(m_Handle, other.m_Handle);
		std::swap(m_EventBase, other.m_EventBase);
	}

	CEventHandlerInstance &CEventHandlerInstance::operator=(CEventHandlerInstance&& other) noexcept
	{
		std::swap(m_Handle, other.m_Handle);
		std::swap(m_EventBase, other.m_EventBase);
		m_EventID = std::move(other.m_EventID);

		return *this;
	}

	esp_event_base_t CEventHandlerInstance::event_base() const
	{
		return m_EventBase;
	}

	//////////////////////////////////////////
	//				 CEventLoop				//
	//////////////////////////////////////////
	CEventLoop::CEventLoop()
		: m_EventHandlerInstances{}
	{
		esp_err_t result = esp_event_loop_create_default();
		if(!success(result))
		{
			LOG_FATAL_ESP("FAILED WITH {}, WHEN TRYING TO CREATE DEFAULT EVENT LOOP", result);
		}
	};

	CEventLoop::~CEventLoop()
	{
		esp_err_t result = esp_event_loop_delete_default();
		if(!success(result))
		{
			LOG_ERROR_ESP("FAILED WITH {}, WHEN TRYING TO DELETE DEFAULT EVENT LOOP", result);
		}
	};

	void CEventLoop::new_event_handler(CEventHandlerInstance &&eventHandlerInstance)
	{
		auto eventBase = eventHandlerInstance.event_base();
		auto[kvPair, emplaced] = m_EventHandlerInstances.try_emplace(eventBase, std::move(eventHandlerInstance));
		if(!emplaced)
		{
			delete_event_handler(eventBase);
			new_event_handler(std::move(kvPair->second));
		}
	};

	void CEventLoop::delete_event_handler(esp_event_base_t eventBase)
	{
		m_EventHandlerInstances.erase(eventBase);
	};

	bool CEventLoop::has_event(esp_event_base_t eventBase) const
	{
		return m_EventHandlerInstances.contains(eventBase);
	};

} // namespace system