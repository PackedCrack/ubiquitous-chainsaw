//
// Created by qwerty on 2024-02-18.
//
#pragma once
#include "defines_wc.hpp"
#include "CRandom.hpp"
#include "hash.hpp"
// third_party
#include "wolfcrypt/ecc.h"
#include "wolfcrypt/asn.h"


namespace security
{
template<typename buffer_t>
concept Buffer = requires(buffer_t buffer)
{
    { buffer.size() } -> std::convertible_to<typename decltype(buffer)::size_type>;
    { buffer.data() } -> std::convertible_to<typename decltype(buffer)::pointer>;
};

class CEccPublicKey
{
public:
    template<typename buffer_t> requires Buffer<std::remove_cvref_t<buffer_t>>
    CEccPublicKey(buffer_t&& derData)
        : m_Key{}
    {
        ASSERT_FMT(derData.size() > 0u, "Tried to create EccPublicKey without data!");
        
        // https://www.wolfssl.com/documentation/manuals/wolfssl/ecc_8h.html#function-wc_ecc_init
        WC_CHECK(wc_ecc_init(&m_Key));
        
        word32 index = 0u;
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ASN.html#function-wc_eccpublickeydecode
        WC_CHECK(wc_EccPublicKeyDecode(derData.data(), &index, &m_Key, assert_down_cast<word32>(derData.size())));
    };
    ~CEccPublicKey();
    CEccPublicKey(const CEccPublicKey& other);
    CEccPublicKey(CEccPublicKey&& other) noexcept;
    CEccPublicKey& operator=(const CEccPublicKey& other);
    CEccPublicKey& operator=(CEccPublicKey&& other) noexcept;
public:
    template<typename buffer_t, typename hash_t>
    requires Buffer<std::remove_cvref_t<buffer_t>> && Hash<std::remove_cvref_t<hash_t>>
    bool verify_hash(buffer_t&& source, hash_t&& hash)
    {
        ASSERT_FMT(source.size() > 0u, "Tried to create verify signature on empty buffer!");
        static constexpr int32_t VALID = 1;
        
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ECC.html#function-wc_ecc_verify_hash
        int32_t result{};
        WC_CHECK(wc_ecc_verify_hash(source.data(), source.size(), hash.data(), hash.size(), &result, &m_Key));
        return result == VALID;
    }
private:
    [[nodiscard]] CEccPublicKey copy() const;
private:
    ecc_key m_Key;
};

class CEccPrivateKey
{
public:
    template<typename buffer_t> requires Buffer<std::remove_cvref_t<buffer_t>>
    explicit CEccPrivateKey(buffer_t&& derData)
            : m_Key{}
    {
        ASSERT_FMT(derData.size() > 0u, "Tried to create EccPublicKey without data!");
        
        // https://www.wolfssl.com/documentation/manuals/wolfssl/ecc_8h.html#function-wc_ecc_init
        WC_CHECK(wc_ecc_init(&m_Key));
        
        word32 index = 0u;
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ASN.html#function-wc_eccprivatekeydecode
        WC_CHECK(wc_EccPrivateKeyDecode(derData.data(), &index, &m_Key, assert_down_cast<word32>(derData.size())));
    };
    ~CEccPrivateKey();
    CEccPrivateKey(const CEccPrivateKey& other);
    CEccPrivateKey(CEccPrivateKey&& other) noexcept;
    CEccPrivateKey& operator=(const CEccPrivateKey& other);
    CEccPrivateKey& operator=(CEccPrivateKey&& other) noexcept;
public:
    template<typename hash_t> requires Hash<std::remove_cvref_t<hash_t>>
    [[nodiscard]] std::vector<byte> sign_hash(CRandom& rng, hash_t&& hash)
    {
        ASSERT_FMT(hash.size() > 0u, "Tried to sign an empty hash!");
        
        std::vector<byte> signature{};
        signature.resize(128u);
        auto bytesWritten = assert_down_cast<word32>(signature.size());
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ECC.html#function-wc_ecc_sign_hash
        WC_CHECK(wc_ecc_sign_hash(hash.data(), hash.size(), signature.data(), &bytesWritten, rng.wc_struct_p(), &m_Key));
        signature.resize(bytesWritten);
        
        return signature;
    }
private:
    [[nodiscard]] CEccPrivateKey copy() const;
private:
    ecc_key m_Key;
};

class CEccKeyPair
{
public:
    explicit CEccKeyPair(CRandom& rng);
    ~CEccKeyPair();
    CEccKeyPair(const CEccKeyPair& other) = delete;
    CEccKeyPair(CEccKeyPair&& other) = delete;
    CEccKeyPair& operator=(const CEccKeyPair& other) = delete;
    CEccKeyPair& operator=(CEccKeyPair&& other) = delete;
public:
    [[nodiscard]] CEccPublicKey public_key();
    [[nodiscard]] CEccPrivateKey private_key();
private:
    ecc_key m_Key;
};
}   // namespace security