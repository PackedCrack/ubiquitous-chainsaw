//
// Created by qwerty on 2024-02-17.
//

#pragma once
// third_party
#define NO_OLD_RNGNAME
#include "wolfcrypt/random.h"
#include "wolfssl/error-ssl.h"


namespace security
{
class CRandom
{
public:
    enum class Error
    {
        constructionFailure,
        destructionFailure
    };
public:
    [[nodiscard]] static std::expected<CRandom, Error> make_rng();
    ~CRandom();
    CRandom(const CRandom& other) = delete;
    CRandom(CRandom&& other) noexcept;
    CRandom& operator=(const CRandom& other) = delete;
    CRandom& operator=(CRandom&& other) noexcept;
private:
    CRandom();
public:
    // TODO:: need better name for this
    WC_RNG& wc_struct();
    WC_RNG* wc_struct_p();
private:
    WC_RNG m_Rng;
};
}   // namespace security