#include "CEventGroup.hpp"
#include "defines.hpp"


namespace sys
{
	CEventGroup::CEventGroup()
		: m_Handle{xEventGroupCreate()}
	{
		if(m_Handle == nullptr)
		{
			LOG_ERROR("NULLPTR RETURNED FROM xEventGroupCreate");
		}
	}

	CEventGroup::~CEventGroup()
	{
		vEventGroupDelete(m_Handle);
	}

	void CEventGroup::set_bit(const EventBits_t bits)
	{
		if(m_Handle)
		{
			xEventGroupSetBits(m_Handle, bits);
		}
		else
		{
			LOG_ERROR("FAILED TO SET EVENT BITS - EVENT GROUP HANDLE IS NULLPTR");
		}
	}

	EventBits_t CEventGroup::wait_for_any_bit(const EventBits_t bits, TickType_t waitTime, BaseType_t clearOnReturn) const
	{
		if(!m_Handle)
		{
			LOG_ERROR("FAILED TO WAIT FOR EVENT BITS - EVENT GROUP HANDLE IS NULLPTR");
			return 0u;
		}
		
		/* From Docs
			Blocks to wait for one or more bits to be set within a previously created event group
		*/
		return xEventGroupWaitBits(m_Handle, bits, clearOnReturn, pdFALSE, waitTime);
	}

	EventBits_t CEventGroup::wait_for_all_bits(const EventBits_t bits, TickType_t waitTime, BaseType_t clearOnReturn) const
	{
		if(!m_Handle)
		{
			LOG_ERROR("FAILED TO WAIT FOR EVENT BITS - EVENT GROUP HANDLE IS NULLPTR");
			return 0u;
		}
		
		/* From Docs
			Blocks to wait for one or more bits to be set within a previously created event group
		*/
		return xEventGroupWaitBits(m_Handle, bits, clearOnReturn, pdTRUE, waitTime);
	}

	std::optional<const EventGroupHandle_t> CEventGroup::handle() const
	{
		if(m_Handle)
			return std::optional<const EventGroupHandle_t>{m_Handle};
		else
			return std::nullopt;
	}

}	// namespace system