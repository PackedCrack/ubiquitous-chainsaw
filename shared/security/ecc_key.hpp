//
// Created by qwerty on 2024-02-18.
//

#pragma once
#include "wc_defines.hpp"
#include "CHash.hpp"
#include "CRandom.hpp"
// third_party
#include "wolfcrypt/ecc.h"
#include "wolfcrypt/asn_public.h"
// clang-format off


// clang-format on
namespace security
{
class CEccPublicKey;
class CEccPrivateKey;
template<typename derived_t>
requires std::same_as<derived_t, CEccPublicKey> || std::same_as<derived_t, CEccPrivateKey>
class IEccKey
{
public:
    // we cannot copy because wolfcrypt requires pointer to non const in order to extract the der format of the ecc_key type.
    IEccKey& operator=(const IEccKey& other) = delete;
    IEccKey(const IEccKey& other) = delete;
protected:
    explicit IEccKey(const std::vector<uint8_t>& derData)
        : m_Key{}
    {
        // cppcheck-suppress ignoredReturnValue
        ASSERT(!derData.empty(), "Tried to create EccKey without data!");

        // https://www.wolfssl.com/documentation/manuals/wolfssl/ecc_8h.html#function-wc_ecc_init
        WC_CHECK(wc_ecc_init(&m_Key));

        decode(derData);
    }
    ~IEccKey()
    {
        if (has_been_moved())
        {
            return;
        }

        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ECC.html#function-wc_ecc_free
        WC_CHECK(wc_ecc_free(&m_Key));
    };
    IEccKey(IEccKey&& other) noexcept
        : m_Key{ other.m_Key }
    {
        static_assert(std::is_trivially_copy_constructible_v<decltype(m_Key)>);
        other.invalidate();
    }
    IEccKey& operator=(IEccKey&& other) noexcept
    {
        if (this != &other)
        {
            static_assert(std::is_trivially_copy_constructible_v<decltype(m_Key)>);
            m_Key = other.m_Key;
            other.invalidate();
        }

        return *this;
    }
public:
    template<typename invokable_t>
    requires std::is_invocable_r_v<bool, invokable_t, std::vector<byte>&&>
    [[nodiscard]] bool write_to_disk(invokable_t&& save)
    {
        return save(static_cast<derived_t*>(this)->to_der());
    }
private:
    void decode(const std::vector<uint8_t>& derData)
    {
        word32 index = 0u;
        if constexpr (std::same_as<derived_t, CEccPublicKey>)
        {
            // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ASN.html#function-wc_eccpublickeydecode
            WC_CHECK(wc_EccPublicKeyDecode(derData.data(), &index, &m_Key, common::assert_down_cast<word32>(derData.size())));
        }
        else
        {
            // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ASN.html#function-wc_eccpublickeydecode
            WC_CHECK(wc_EccPrivateKeyDecode(derData.data(), &index, &m_Key, common::assert_down_cast<word32>(derData.size())));
        }
    }
    void invalidate()
    {
        m_Key.heap = nullptr;
        m_Key.dp = nullptr;
    }
    [[nodiscard]] bool has_been_moved() { return m_Key.heap == nullptr && m_Key.dp == nullptr; }
protected:
    ecc_key m_Key;
};
/**
 *
 */
class CEccPublicKey : public IEccKey<CEccPublicKey>
{
public:
    explicit CEccPublicKey(const std::vector<uint8_t>& derData);
    ~CEccPublicKey() = default;
    CEccPublicKey(const CEccPublicKey& other) = delete;
    CEccPublicKey(CEccPublicKey&& other) = default;
    CEccPublicKey& operator=(const CEccPublicKey& other) = delete;
    CEccPublicKey& operator=(CEccPublicKey&& other) noexcept;
public:
    template<typename buffer_t, typename hash_t>
    requires common::buffer<std::remove_cvref_t<buffer_t>> && hash<std::remove_cvref_t<hash_t>>
    bool verify_hash(buffer_t&& source, hash_t&& hash)
    {
        ASSERT(source.size() > 0u, "Tried to create verify signature on empty buffer!");
        static constexpr int32_t VALID = 1;

        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ECC.html#function-wc_ecc_verify_hash
        int result{};
        WC_CHECK(wc_ecc_verify_hash(source.data(),
                                    common::assert_down_cast<word32>(source.size()),
                                    hash.data(),
                                    common::assert_down_cast<word32>(hash.size()),
                                    &result,
                                    &m_Key));
        return result == VALID;
    }
    [[nodiscard]] std::vector<byte> to_der();
};
/**
 *
 */
class CEccPrivateKey : public IEccKey<CEccPrivateKey>
{
public:
    explicit CEccPrivateKey(const std::vector<uint8_t>& derData);
    ~CEccPrivateKey() = default;
    CEccPrivateKey(const CEccPrivateKey& other) = delete;
    CEccPrivateKey(CEccPrivateKey&& other) = default;
    CEccPrivateKey& operator=(const CEccPrivateKey& other) = delete;
    CEccPrivateKey& operator=(CEccPrivateKey&& other) noexcept;
public:
    template<typename hash_t>
    requires hash<std::remove_cvref_t<hash_t>>
    [[nodiscard]] std::vector<byte> sign_hash(CRandom& rng, hash_t&& hash)
    {
        ASSERT(hash.size() > 0u, "Tried to sign an empty hash!");

        std::vector<byte> signature{};
        signature.resize(128u);
        auto bytesWritten = common::assert_down_cast<word32>(signature.size());
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ECC.html#function-wc_ecc_sign_hash
        WC_CHECK(wc_ecc_sign_hash(hash.data(),
                                  common::assert_down_cast<word32>(hash.size()),
                                  signature.data(),
                                  &bytesWritten,
                                  rng.wc_struct_p(),
                                  &m_Key));
        signature.resize(bytesWritten);

        return signature;
    }
    [[nodiscard]] std::vector<byte> to_der();
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
template<typename key_t, typename invokable_t>
requires std::is_invocable_r_v<std::expected<std::vector<byte>, std::string>, invokable_t>
[[nodiscard]] std::optional<key_t> make_ecc_key(invokable_t&& load)
{
    auto expected = load();
    if (expected)
    {
        return std::optional<key_t>{ *expected };
    }
    else
    {
        LOG_ERROR_FMT("Failed to create ecc key: \"{}\"", expected.error());
        return std::nullopt;
    }
}
}    // namespace security
