#pragma once
#include "CProfileCache.hpp"


namespace ble
{
class CProfileCacheBuilder
{
public:
	[[nodiscard]] CProfileCacheBuilder& add_whoami();
	[[nodiscard]] CProfileCache build();
private:
	std::map<std::string_view, CProfile> m_Profiles;
};
}	// namespace ble