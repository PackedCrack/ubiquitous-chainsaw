#pragma once
#include <memory>


template<typename derived_t>
class SharableThis
{
public:
	SharableThis()
		: m_pSelf{ std::make_shared<derived_t*>(static_cast<derived_t*>(this)) }
	{}
	~SharableThis() = default;
	SharableThis(const SharableThis& other)
		: m_pSelf{ std::make_shared<derived_t*>(static_cast<derived_t*>(this)) }
	{}
	SharableThis(SharableThis&& other)
		: m_pSelf{ std::move(other.m_pSelf) }
	{
		*m_pSelf = static_cast<derived_t*>(this);
	}
	SharableThis& operator=(const SharableThis& other) = delete;
	SharableThis& operator=(SharableThis&& other) = delete;
protected:
	void store_this(){ m_pSelf = std::make_shared<derived_t*>(static_cast<derived_t*>(this)); };
	[[nodiscard]] std::weak_ptr<derived_t*> share_this(){ return m_pSelf; }
protected:
	std::shared_ptr<derived_t*> m_pSelf;
};