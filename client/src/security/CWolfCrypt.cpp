//
// Created by qwerty on 2024-02-17.
//
#include "CWolfCrypt.hpp"
#include "defines.hpp"
// third party
#include "wolfcrypt/types.h"


namespace
{
constexpr int32_t SUCCESS = 0;
}   // namespace

namespace security
{
std::expected<CWolfCrypt*, CWolfCrypt::Error> CWolfCrypt::instance()
{
    try
    {
        static CWolfCrypt instance{};
        return std::expected<CWolfCrypt*, CWolfCrypt::Error>{ &instance };
    }
    catch(const std::runtime_error& err)
    {
        LOG_ERROR_FMT("{}", err.what());
        return std::unexpected{ CWolfCrypt::Error::constructorFailure };
    }
}
CWolfCrypt::CWolfCrypt()
{
    int32_t result = wolfCrypt_Init();
    if(result != SUCCESS)
        throw std::runtime_error{ std::format("Failed to initialize wolfcrypt.. Failed with: \"{}\"", result) };
    
}
CWolfCrypt::~CWolfCrypt()
{
    int32_t result = wolfCrypt_Cleanup();
    if(result != SUCCESS)
    {
        LOG_ERROR_FMT("Failed to cleanup wolfcrypt.. Failed with: \"{}\"", result);
    }
}
}   // namespace security