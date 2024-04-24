#include "CProfileCache.hpp"
#include "../ble_common.hpp"
// esp
#include "host/ble_hs.h"


namespace
{
[[nodiscard]] constexpr ble_gatt_svc_def end_of_array()
{
    return ble_gatt_svc_def {
        .type = BLE_GATT_SVC_TYPE_END,
        .uuid = nullptr,
        .includes = nullptr,
        .characteristics = nullptr
    };
}
}   // namespace
namespace ble
{
Result<CProfileCache, CProfileCache::Error> CProfileCache::make_profile_cache(std::map<CProfileCache::KeyType, std::shared_ptr<Profile>>&& profiles)
{
    try
    {
        CProfileCache cache{ std::move(profiles) };
        return Result<CProfileCache, CProfileCache::Error>{
            .value = std::make_optional<CProfileCache>(std::move(cache)),
            .error = Error::none
        };
    }
    catch(const CProfileCache::Error& err)
    {
        using ErrorCode = CProfileCache::Error;

        if(err == ErrorCode::invalidResource)
        {
            return Result<CProfileCache, CProfileCache::Error>{
                .value = std::nullopt,
                .error = Error::invalidResource
            };
        }
        else if(err == ErrorCode::outOfHeapMemory)
        {
            return Result<CProfileCache, CProfileCache::Error>{
                .value = std::nullopt,
                .error = Error::outOfHeapMemory
            };
        }
        else
        {
            LOG_FATAL_FMT("Unknown enum value thrown by CProfileCache's constructior: \"{}\"", static_cast<int32_t>(err));
        }
    }

    __builtin_unreachable();
}
CProfileCache::CProfileCache(std::map<CProfileCache::KeyType, std::shared_ptr<Profile>>&& profiles)
    : m_Profiles{ std::move(profiles) }
{
    for(auto&& kvPair : m_Profiles)
    {
        Profile& profile = *kvPair.second;
        std::visit([this]<typename profile_t>(profile_t&& profile){ m_Services.emplace_back(profile.as_nimble_service()); }, profile);
    }
    m_Services.emplace_back(end_of_array());


    // register m_Services
    NimbleErrorCode result = NimbleErrorCode{ ble_gatts_count_cfg(m_Services.data()) };
    if(result == NimbleErrorCode::invalidArguments)
    {
        throw Error::invalidResource;
    }
    else if(result != NimbleErrorCode::success)
    {
        LOG_FATAL_FMT("Unknown error when calling ble_gatts_count_cfg inside CProfileCache's constructor: \"{}\"", 
                        nimble_error_to_string(result));
    }

    result = NimbleErrorCode{ ble_gatts_add_svcs(m_Services.data()) };
    if (result == NimbleErrorCode::resourceExhaustion) 
    {
        throw Error::outOfHeapMemory;
    }
    else if(result != NimbleErrorCode::success)
    {
        LOG_FATAL_FMT("Unknown error when calling ble_gatts_add_svcs inside CProfileCache's constructor: \"{}\"", 
                        nimble_error_to_string(result));
    }
}
}   // namespace ble