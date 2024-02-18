//
// Created by qwerty on 2024-02-18.
//

#pragma once
#include "defines_wc.hpp"
#include "CHash.hpp"
#include "CRandom.hpp"
// third_party
#include "wolfcrypt/ecc.h"
#include "wolfcrypt/asn_public.h"


namespace security
{
template<typename derived_t>
class IEccKey
{
protected:
    explicit IEccKey(const std::vector<uint8_t>& derData)
        : m_Key{}
    {
        ASSERT_FMT(!derData.empty(), "Tried to create EccKey without data!");
        
        // https://www.wolfssl.com/documentation/manuals/wolfssl/ecc_8h.html#function-wc_ecc_init
        WC_CHECK(wc_ecc_init(&m_Key));
        
        static_cast<derived_t*>(this)->decode(derData);
    }
    virtual ~IEccKey()
    {
        if(has_been_moved())
            return;
        
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ECC.html#function-wc_ecc_free
        WC_CHECK(wc_ecc_free(&m_Key));
    };
    IEccKey(const IEccKey& other) = default;
    IEccKey(IEccKey&& other) noexcept
            : m_Key{ other.m_Key }
    {
        static_assert(std::is_trivially_copy_constructible_v<decltype(m_Key)>);
        other.invalidate();
    }
    IEccKey& operator=(const IEccKey& other) = default;
    IEccKey& operator=(IEccKey&& other) noexcept
    {
        if(this != &other)
        {
            static_assert(std::is_trivially_copy_constructible_v<decltype(m_Key)>);
            m_Key = other.m_Key;
            other.invalidate();
        }
        
        return *this;
    }
protected:
    void invalidate()
    {
        m_Key.heap = nullptr;
        m_Key.dp = nullptr;
    }
    [[nodiscard]] bool has_been_moved()
    {
        return m_Key.heap == nullptr && m_Key.dp == nullptr;
    }
protected:
    ecc_key m_Key;
};

/**
 *
 */
class CEccPublicKey : public IEccKey<CEccPublicKey>
{
    friend IEccKey<CEccPublicKey>;
public:
    explicit CEccPublicKey(const std::vector<uint8_t>& derData);
    ~CEccPublicKey() override = default;
    CEccPublicKey(const CEccPublicKey& other);
    CEccPublicKey(CEccPublicKey&& other) = default;
    CEccPublicKey& operator=(const CEccPublicKey& other);
    CEccPublicKey& operator=(CEccPublicKey&& other) = default;
public:
    template<typename buffer_t, typename hash_t>
    requires common::Buffer<std::remove_cvref_t<buffer_t>> && Hash<std::remove_cvref_t<hash_t>>
    bool verify_hash(buffer_t&& source, hash_t&& hash)
    {
        ASSERT_FMT(source.size() > 0u, "Tried to create verify signature on empty buffer!");
        static constexpr int32_t VALID = 1;
    
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ECC.html#function-wc_ecc_verify_hash
        int32_t result{};
        WC_CHECK(wc_ecc_verify_hash(
                source.data(),
                common::assert_down_cast<word32>(source.size()),
                hash.data(),
                common::assert_down_cast<word32>(hash.size()),
                &result,
                &m_Key));
        return result == VALID;
    }
private:
    void decode(const std::vector<uint8_t>& derData);
    void copy(ecc_key cpy);
};

/**
 *
 */
class CEccPrivateKey : public IEccKey<CEccPrivateKey>
{
    friend IEccKey<CEccPrivateKey>;
public:
    explicit CEccPrivateKey(const std::vector<uint8_t>& derData);
    ~CEccPrivateKey() override = default;
    CEccPrivateKey(const CEccPrivateKey& other);
    CEccPrivateKey(CEccPrivateKey&& other) = default;
    CEccPrivateKey& operator=(const CEccPrivateKey& other);
    CEccPrivateKey& operator=(CEccPrivateKey&& other) = default;
public:
    template<typename hash_t> requires Hash<std::remove_cvref_t<hash_t>>
    [[nodiscard]] std::vector<byte> sign_hash(CRandom& rng, hash_t&& hash)
    {
        ASSERT_FMT(hash.size() > 0u, "Tried to sign an empty hash!");
        
        std::vector<byte> signature{};
        signature.resize(128u);
        auto bytesWritten = common::assert_down_cast<word32>(signature.size());
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ECC.html#function-wc_ecc_sign_hash
        WC_CHECK(wc_ecc_sign_hash(
                hash.data(),
                common::assert_down_cast<word32>(hash.size()),
                signature.data(),
                &bytesWritten,
                rng.wc_struct_p(),
                &m_Key));
        signature.resize(bytesWritten);
        
        return signature;
    }
private:
    void decode(const std::vector<uint8_t>& derData);
    void copy(ecc_key cpy);
};

/**
 *
 */
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