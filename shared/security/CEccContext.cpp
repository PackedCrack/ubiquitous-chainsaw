//
// Created by qwerty on 2024-02-19.
//
#include "CEccContext.hpp"
#include "defines_wc.hpp"


namespace
{
[[nodiscard]] ecEncCtx* make_context(security::CRandom& rng)
{
    // REQ_RESP_SERVER
    // https://www.wolfssl.com/documentation/manuals/wolfssl/ecc_8h.html#function-wc_ecc_ctx_new
    ecEncCtx* pContext = wc_ecc_ctx_new(REQ_RESP_CLIENT, rng.wc_struct_p());
    ASSERT(pContext != nullptr, "Failed to allocated Ecc Context!");
    
    return pContext;
}
[[nodiscard]] byte enc_alg_to_wc_constant(security::EcEncAlg encAlg)
{
    using namespace security;
    
    UNHANDLED_CASE_PROTECTION_ON
    switch(encAlg)
    {
        case EcEncAlg::AES_128_CBC:
            return ecAES_128_CBC;
        case EcEncAlg::AES_128_CTR:
            return ecAES_128_CTR;
        case EcEncAlg::AES_256_CBC:
            return ecAES_256_CBC;
        case EcEncAlg::AES_256_CTR:
            return ecAES_256_CTR;
    }
    UNHANDLED_CASE_PROTECTION_OFF
    std::unreachable();
}
[[nodiscard]] byte kdf_alg_to_wc_constant(security::EcKdfAlg kdfAlg)
{
    using namespace security;
    
    UNHANDLED_CASE_PROTECTION_ON
    switch(kdfAlg)
    {
        case EcKdfAlg::HKDF_SHA1:
            return ecHKDF_SHA1;
        case EcKdfAlg::HKDF_SHA256:
            return ecHKDF_SHA256;
        case EcKdfAlg::KDF_SHA1:
            return ecKDF_SHA1;
        case EcKdfAlg::KDF_SHA256:
            return ecKDF_SHA256;
        case EcKdfAlg::KDF_X963_SHA1:
            return ecKDF_X963_SHA1;
        case EcKdfAlg::KDF_X963_SHA256:
            return ecKDF_X963_SHA256;
    }
    UNHANDLED_CASE_PROTECTION_OFF
    std::unreachable();
}
[[nodiscard]] byte mac_alg_to_wc_constant(security::EcMacAlg macAlg)
{
    using namespace security;
    
    UNHANDLED_CASE_PROTECTION_ON
    switch(macAlg)
    {
        case EcMacAlg::HMAC_SHA1:
            return ecHMAC_SHA1;
        case EcMacAlg::HMAC_SHA256:
            return ecHMAC_SHA256;
    }
    UNHANDLED_CASE_PROTECTION_OFF
    std::unreachable();
}
}   // namespace

namespace security
{
CEccContext::CEccContext(CRandom& rng)
    : m_pContext{ make_context(rng) }
    , m_EncAlg{ EcEncAlg::AES_128_CBC }
    , m_KdfAlg{ EcKdfAlg::HKDF_SHA256 }
    , m_MacAlg{ EcMacAlg::HMAC_SHA256 }
{
    set_context_algorithms();
}
CEccContext::CEccContext(EcEncAlg encAlg, EcKdfAlg kdfAlg, EcMacAlg macAlg, CRandom& rng)
    : m_pContext{ make_context(rng) }
    , m_EncAlg{ encAlg }
    , m_KdfAlg{ kdfAlg }
    , m_MacAlg{ macAlg }
{
    set_context_algorithms();
}
CEccContext::~CEccContext()
{
    if(m_pContext == nullptr)
        return;
    
    // https://www.wolfssl.com/documentation/manuals/wolfssl/ecc_8h.html#function-wc_ecc_ctx_free
    wc_ecc_ctx_free(m_pContext);
}
CEccContext::CEccContext(CEccContext&& other) noexcept
        : m_pContext{ nullptr }
        , m_EncAlg{ other.m_EncAlg }
        , m_KdfAlg{ other.m_KdfAlg }
        , m_MacAlg{ other.m_MacAlg }
{
    std::swap(m_pContext, other.m_pContext);
}
CEccContext& CEccContext::operator=(CEccContext&& other) noexcept
{
    if(this != &other)
    {
        std::swap(m_pContext, other.m_pContext);
        m_EncAlg = other.m_EncAlg;
        m_KdfAlg = other.m_KdfAlg;
        m_MacAlg = other.m_MacAlg;
    }
    
    return *this;
}
void CEccContext::set_context_algorithms()
{
    // https://www.wolfssl.com/documentation/manuals/wolfssl/ecc_8h.html#function-wc_ecc_ctx_set_algo
    WC_CHECK(wc_ecc_ctx_set_algo(
            m_pContext,
            enc_alg_to_wc_constant(m_EncAlg),
            kdf_alg_to_wc_constant(m_KdfAlg),
            mac_alg_to_wc_constant(m_MacAlg)));
}
}   // security