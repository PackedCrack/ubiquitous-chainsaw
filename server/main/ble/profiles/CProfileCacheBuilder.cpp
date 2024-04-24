#include "CProfileCacheBuilder.hpp"
#include "CWhoAmI.hpp"


namespace ble
{
CProfileCacheBuilder& CProfileCacheBuilder::add_whoami() 
{ 
    if(!try_emplace_profile<CWhoAmI>(CProfileCache::KEY_WHOAMI))
    {
        LOG_FATAL("CProfileCacheBuilder failed to add CWhoAmI profile to the cache.");
    }

    return *this;
}
CProfileCacheBuilder& CProfileCacheBuilder::add_whereami() 
{ 
    if(!try_emplace_profile<CWhereAmI>(CProfileCache::KEY_WHEREAMI))
    {
        LOG_FATAL("CProfileCacheBuilder failed to add CWhereAmI profile to the cache.");
    }

    return *this;
}
std::unique_ptr<CProfileCache> CProfileCacheBuilder::build() 
{ 
    using Error = CProfileCache::Error;


    Result<CProfileCache, Error> result = CProfileCache::make_profile_cache(std::move(m_Profiles));
    if(result.error == Error::none)
    {
    	ASSERT(result.value, "Expected a value in result when error code is none..");
    	return std::make_unique<CProfileCache>(std::move(result.value.value()));
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
}   // namespace ble