//
// Created by qwerty on 2024-02-17.
//
#include "CRandom.hpp"
#include "common.hpp"
#include "defines_wc.hpp"


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
        std::unexpected{ Error::construction };
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
    
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__Random.html#function-wc_freerng
    WC_CHECK(wc_FreeRng(&m_Rng));
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
std::expected<std::vector<uint8_t>, CRandom::Error> CRandom::generate_block(size_t size)
{
    static constexpr WCResult SUCCESS = 0;
    std::vector<uint8_t> dataBlock{};
    dataBlock.resize(size);
    
    WCResult code = wc_RNG_GenerateBlock(&m_Rng, dataBlock.data(), common::assert_down_cast<word32>(dataBlock.size()));
    if(code == SUCCESS)
        return dataBlock;
    else
        return std::unexpected{ Error::blockGeneration };
}
WC_RNG& CRandom::wc_struct()
{
    return m_Rng;
}
WC_RNG* CRandom::wc_struct_p()
{
    return &m_Rng;
}
}   // namespace security