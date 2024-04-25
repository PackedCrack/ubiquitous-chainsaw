//
// Created by qwerty on 2024-02-19.
//

#pragma once
#include "CRandom.hpp"
// third-party
// #define HAVE_ECC_ENCRYPT - required option for encrytion. But simple encrytion with public key does not seem to exist
#include "wolfssl/options.h"
#include "wolfcrypt/ecc.h"
namespace security
{
enum class EcEncAlg
{
    AES_128_CBC = 1,
    AES_256_CBC = 2,
    AES_128_CTR = 3,
    AES_256_CTR = 4
};
enum class EcKdfAlg
{
    HKDF_SHA256 = 1,
    HKDF_SHA1 = 2,
    KDF_X963_SHA1 = 3,
    KDF_X963_SHA256 = 4,
    KDF_SHA1 = 5,
    KDF_SHA256 = 6
};
enum class EcMacAlg
{
    HMAC_SHA256 = 1,
    HMAC_SHA1 = 2
};
class CEccContext
{
public:
    explicit CEccContext(CRandom& rng);
    CEccContext(EcEncAlg encAlg, EcKdfAlg kdfAlg, EcMacAlg macAlg, CRandom& rng);
    ~CEccContext();
    CEccContext(const CEccContext& other) = delete;
    CEccContext(CEccContext&& other) noexcept;
    CEccContext& operator=(const CEccContext& other) = delete;
    CEccContext& operator=(CEccContext&& other) noexcept;
private:
    void set_context_algorithms();
    /*set_algo()
    {
        // https://www.wolfssl.com/documentation/manuals/wolfssl/ecc_8h.html#function-wc_ecc_ctx_set_algo
        int wc_ecc_ctx_set_algo(
                ecEncCtx * ctx,
                byte encAlgo,
                byte kdfAlgo,
                byte macAlgo
        )
    }
    
    get_salt()
    {
        // https://www.wolfssl.com/documentation/manuals/wolfssl/ecc_8h.html#function-wc_ecc_ctx_get_own_salt
        const byte * wc_ecc_ctx_get_own_salt(
                ecEncCtx *
        )
    }
    
    set_peer_salt()
    {
        // https://www.wolfssl.com/documentation/manuals/wolfssl/ecc_8h.html#function-wc_ecc_ctx_set_peer_salt
        int wc_ecc_ctx_set_peer_salt(
                ecEncCtx * ctx,
                const byte * salt
        )
    }
    
    set_kdf_salt()
    {
        // https://www.wolfssl.com/documentation/manuals/wolfssl/ecc_8h.html#function-wc_ecc_ctx_set_kdf_salt
        int wc_ecc_ctx_set_kdf_salt(
                ecEncCtx * ctx,
                const byte * salt,
                word32 len
        )
    }
    
    set_info()
    {
        // https://www.wolfssl.com/documentation/manuals/wolfssl/ecc_8h.html#function-wc_ecc_ctx_set_info
        int wc_ecc_ctx_set_info(
                ecEncCtx * ctx,
                const byte * info,
                int sz
        )
    }*/
private:
    ecEncCtx* m_pContext = nullptr;
    EcEncAlg m_EncAlg;
    EcKdfAlg m_KdfAlg;
    EcMacAlg m_MacAlg;
};
}    // namespace security
