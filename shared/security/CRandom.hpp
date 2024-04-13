//
// Created by qwerty on 2024-02-17.
//

#pragma once
// third_party
#include "wolfcrypt/random.h"
#include "wolfssl/error-ssl.h"


namespace security
{
class CRandom
{
public:
    enum class Error
    {
        construction,
        blockGeneration
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
    [[nodiscard]] std::expected<std::vector<uint8_t>, Error> generate_block(size_t size);
    // TODO:: need better name for this
    [[nodiscard]] WC_RNG& wc_struct();
    [[nodiscard]] WC_RNG* wc_struct_p();
private:
    WC_RNG m_Rng;
};
}   // namespace security