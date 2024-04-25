//
// Created by qwerty on 2024-02-17.
//
#include "CWolfCrypt.hpp"
#include "wc_defines.hpp"
// third party
#include "wolfcrypt/types.h"
namespace security
{
std::expected<CWolfCrypt*, CWolfCrypt::Error> CWolfCrypt::instance()
{
    try
    {
        static CWolfCrypt obj{};
        return std::expected<CWolfCrypt*, CWolfCrypt::Error>{ &obj };
    }
    catch (const std::runtime_error& err)
    {
        LOG_ERROR_FMT("{}", err.what());
        return std::unexpected{ CWolfCrypt::Error::constructorFailure };
    }
}
CWolfCrypt::CWolfCrypt()
{
    WCResult result = wolfCrypt_Init();
    if (result != WC_SUCCESS)
    {
        throw std::runtime_error{ std::format("Failed to initialize wolfcrypt.. Failed with: \"{}\"", result) };
    }
}
CWolfCrypt::~CWolfCrypt()
{
    WCResult result = wolfCrypt_Cleanup();
    if (result != WC_SUCCESS)
    {
        LOG_ERROR_FMT("Failed to cleanup wolfcrypt.. Failed with: \"{}\"", result);
    }
}
}    // namespace security
