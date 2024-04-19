#pragma once
#include "CProfileCache.hpp"


namespace ble
{
class CProfileCacheBuilder
{
public:
	[[nodiscard]] CProfileCacheBuilder& add_whoami();
	[[nodiscard]] CProfileCacheBuilder& add_whereami();
	[[nodiscard]] std::unique_ptr<CProfileCache> build();
private:
	template<typename profile_t, typename... ctor_args_t>
	requires NimbleProfile<profile_t>
	[[nodiscard]] bool try_emplace_profile(CProfileCache::KeyType key, ctor_args_t&&... args)
	{
		auto[iter, emplaced] = m_Profiles.try_emplace(key, make_profile<profile_t>(std::forward<ctor_args_t>(args)...));
		if(emplaced)
		{
			std::shared_ptr<Profile>& pVariantProfile = iter->second;
			profile_t& profile = std::get<profile_t>(*pVariantProfile);
			profile.register_with_nimble(pVariantProfile);
		}

		return emplaced;
	}
private:
	std::map<std::string_view, std::shared_ptr<Profile>> m_Profiles;
};
}	// namespace ble