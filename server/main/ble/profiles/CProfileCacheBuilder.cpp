#include "CProfileCacheBuilder.hpp"


namespace ble
{
CProfileCacheBuilder& CProfileCacheBuilder::add_whoami() 
{ 
	auto[iter, emplaced] = m_Profiles.try_emplace(CProfileCache::KEY_WHOAMI, make_profile<CWhoAmI>());
	if(!emplaced)
	{
		LOG_FATAL("CPRofilesBuilder failed to att CWhoAmI profile to cache.");
	}

	return *this;
}
CProfileCache CProfileCacheBuilder::build() 
{ 
	using Error = CProfileCache::Error;


	Result<CProfileCache, Error> result = CProfileCache::make_profile_cache(std::move(m_Profiles));
	if(result.error == Error::none)
	{
		ASSERT(result.value, "Expected a value in result when error code is none..");
		return CProfileCache{ std::move(result.value.value()) };
	}
	else
	{
		if(result.error == Error::outOfHeapMemory)
		{
			// TODO:: This is not fatal and should be handled somehow
			LOG_FATAL("CProfileBuilder failed to build CProfileCache because heap memory is full!");
		}
		else
		{
			ASSERT(result.error == Error::invalidResource, "There should only be three Error alternatives.. Expected invalidResource.");
			LOG_FATAL("Invalid arguments passed to CProfileConstructor from CProfileBuilder");
		}
	}

	__builtin_unreachable();
}
}	// namespace ble