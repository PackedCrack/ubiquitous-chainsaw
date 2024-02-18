//
// Created by qwerty on 2024-02-17.
//
#include "CRandom.hpp"
#include "defines.hpp"


namespace
{
constexpr int32_t SUCCESS = 0;

[[nodiscard]] std::string_view init_err_msg(int32_t code)
{
    switch(code)
    {
        case MEMORY_E:
        {
            static constexpr std::string_view errMsg = "XMALLOC failed";
            return errMsg;
        }
        case WINCRYPT_E:
        {
            static constexpr std::string_view errMsg = "wc_GenerateSeed: failed to acquire context";
            return errMsg;
        }
        case CRYPTGEN_E:
        {
            static constexpr std::string_view errMsg = "wc_GenerateSeed: failed to get random";
            return errMsg;
        }
        case BAD_FUNC_ARG:
        {
            static constexpr std::string_view errMsg = "wc_RNG_GenerateBlock input is null or sz exceeds MAX_REQUEST_LEN";
            return errMsg;
        }
        case DRBG_CONT_FIPS_E:
        {
            static constexpr std::string_view errMsg = "wc_RNG_GenerateBlock: Hash_gen returned DRBG_CONT_FAILURE";
            return errMsg;
        }
        case RNG_FAILURE_E:
        {
            static constexpr std::string_view errMsg = "wc_RNG_GenerateBlock: Default error. rng’s status originally not ok, or set to DRBG_FAILED";
            return errMsg;
        }
    }
    
    std::unreachable();
}
[[nodiscard]] std::string_view free_err_msg(int32_t code)
{
    switch(code)
    {
        case RNG_FAILURE_E:
        {
            static constexpr std::string_view errMsg = "Failed to deallocated drbg";
            return errMsg;
        }
        case BAD_FUNC_ARG:
        {
            static constexpr std::string_view errMsg = "rng or rng->drgb null";
            return errMsg;
        }
    }
    
    std::unreachable();
}
} // namespace

namespace security
{
std::expected<CRandom, CRandom::Error> CRandom::make_rng()
{
    try
    {
        return CRandom{};
    }
    catch(const std::runtime_error& err)
    {
        LOG_ERROR_FMT("Failed to create RNG: \"{}\"", err.what());
        std::unexpected{ Error::constructionFailure };
    }
}
CRandom::CRandom()
    : m_Rng{}
{
    
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__Random.html#function-wc_initrng
    int32_t result = wc_InitRng(&m_Rng);
    if(result != SUCCESS)
        throw std::runtime_error{ init_err_msg(result).data() };
}
CRandom::~CRandom()
{
    if(m_Rng.heap == nullptr)   // has been moved
        return;
    
    int32_t result = wc_FreeRng(&m_Rng);
    if(result != SUCCESS)
        throw std::runtime_error{ free_err_msg(result).data() };
}
CRandom::CRandom(CRandom&& other) noexcept
    : m_Rng{ other.m_Rng }
{
    static_assert(std::is_trivially_copy_constructible_v<decltype(m_Rng)>);
    
    other.m_Rng.heap = nullptr;
    other.m_Rng.drbg = nullptr;
}
CRandom& CRandom::operator=(CRandom&& other) noexcept
{
    static_assert(std::is_trivially_copy_constructible_v<decltype(m_Rng)>);
    
    if(this != &other)
    {
        m_Rng = other.m_Rng;
        other.m_Rng.heap = nullptr;
        other.m_Rng.drbg = nullptr;
    }
    
    return *this;
}
CRandom::operator WC_RNG*()
{
    return &m_Rng;
}
}   // namespace security