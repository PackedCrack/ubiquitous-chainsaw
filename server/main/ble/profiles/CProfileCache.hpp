#pragma once
#include "../../server_common.hpp"
#include "profiles.hpp"
// std
#include <memory>
#include <map>
#include <concepts>
// esp
#include "host/ble_gatt.h"
//
//
//
//
namespace ble
{
template<typename profile_t>
concept NimbleProfile = requires(profile_t profile, const std::shared_ptr<Profile>& pProfile) {
    { profile.as_nimble_service() } -> std::convertible_to<ble_gatt_svc_def>;
    { profile.register_with_nimble(pProfile) };
};
template<typename profile_t, typename... ctor_args_t>
requires NimbleProfile<profile_t>
[[nodiscard]] std::shared_ptr<Profile> make_profile(ctor_args_t&&... args)
{
    return std::make_shared<Profile>(profile_t{ std::forward<ctor_args_t>(args)... });
}
class CProfileCache
{
public:
    using KeyType = std::string_view;
    enum class Error
    {
        none,
        outOfHeapMemory,
        invalidResource
    };
    static constexpr KeyType KEY_WHOAMI = "whoami";
    static constexpr KeyType KEY_WHEREAMI = "whereami";
    static constexpr KeyType KEY_RANGE = "range";
public:
    [[nodiscard]] static Result<CProfileCache, CProfileCache::Error>
        make_profile_cache(std::map<KeyType, std::shared_ptr<Profile>>&& profiles);
    ~CProfileCache() = default;
    CProfileCache(const CProfileCache& other) = default;
    CProfileCache(CProfileCache&& other) = default;
    CProfileCache& operator=(const CProfileCache& other) = default;
    CProfileCache& operator=(CProfileCache&& other) = default;
private:
    explicit CProfileCache(std::map<KeyType, std::shared_ptr<Profile>>&& profiles);
private:
    std::map<KeyType, std::shared_ptr<Profile>> m_Profiles;
    std::vector<ble_gatt_svc_def> m_Services;
};
}    // namespace ble
