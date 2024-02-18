//
// Created by qwerty on 2024-02-18.
//

#pragma once
#include "common.hpp"
#include "defines_wc.hpp"
#define NO_OLD_WC_NAMES
#include "wolfcrypt/sha.h"
#include "wolfssl/wolfcrypt/asn.h"


namespace security
{
struct Sha
{
    using buffer_t = std::vector<byte>;
    
    static constexpr std::string_view HASH_NAME = "SHA";
    static constexpr size_t HASH_SIZE = WC_SHA_DIGEST_SIZE;
    
    template<typename buffer_ref_t>
    requires std::is_same_v<std::remove_cvref_t<buffer_ref_t>, buffer_t>
    [[nodiscard]] static buffer_t hash(std::string_view text, buffer_ref_t&& buffer)
    {
        static_assert(alignof(byte) == alignof(decltype(text)::value_type));
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_shahash
        WC_CHECK(wc_ShaHash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
        return buffer;
    }
};
struct Sha2_224
{
    using buffer_t = std::vector<byte>;
    
    static constexpr std::string_view HASH_NAME = "SHA2-224";
    static constexpr size_t HASH_SIZE = WC_SHA224_DIGEST_SIZE;
    
    template<typename buffer_ref_t>
    requires std::is_same_v<std::remove_cvref_t<buffer_ref_t>, buffer_t>
    [[nodiscard]] static buffer_t hash(std::string_view text, buffer_ref_t&& buffer)
    {
        static_assert(alignof(byte) == alignof(decltype(text)::value_type));
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_sha224hash
        WC_CHECK(wc_Sha224Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
        return buffer;
    }
};
struct Sha2_256
{
    using buffer_t = std::vector<byte>;
    
    static constexpr std::string_view HASH_NAME = "SHA2-256";
    static constexpr size_t HASH_SIZE = WC_SHA256_DIGEST_SIZE;
    
    template<typename buffer_ref_t>
    requires std::is_same_v<std::remove_cvref_t<buffer_ref_t>, buffer_t>
    [[nodiscard]] static buffer_t hash(std::string_view text, buffer_ref_t&& buffer)
    {
        static_assert(alignof(byte) == alignof(decltype(text)::value_type));
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_sha256hash
        WC_CHECK(wc_Sha256Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
        return buffer;
    }
};
struct Sha2_384
{
    using buffer_t = std::vector<byte>;
    
    static constexpr std::string_view HASH_NAME = "SHA2-384";
    static constexpr size_t HASH_SIZE = WC_SHA384_DIGEST_SIZE;
    
    template<typename buffer_ref_t>
    requires std::is_same_v<std::remove_cvref_t<buffer_ref_t>, buffer_t>
    [[nodiscard]] static buffer_t hash(std::string_view text, buffer_ref_t&& buffer)
    {
        static_assert(alignof(byte) == alignof(decltype(text)::value_type));
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_sha384hash
        WC_CHECK(wc_Sha384Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
        return buffer;
    }
};
struct Sha2_512
{
    using buffer_t = std::vector<byte>;
    
    static constexpr std::string_view HASH_NAME = "SHA2-512";
    static constexpr size_t HASH_SIZE = WC_SHA512_DIGEST_SIZE;
    
    template<typename buffer_ref_t>
    requires std::is_same_v<std::remove_cvref_t<buffer_ref_t>, buffer_t>
    [[nodiscard]] static buffer_t hash(std::string_view text, buffer_ref_t&& buffer)
    {
        static_assert(alignof(byte) == alignof(decltype(text)::value_type));
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_sha512hash
        WC_CHECK(wc_Sha512Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
        return buffer;
    }
};

struct Sha3_224
{
    using buffer_t = std::vector<byte>;
    
    static constexpr std::string_view HASH_NAME = "SHA3-224";
    static constexpr size_t HASH_SIZE = WC_SHA224_DIGEST_SIZE;
    
    template<typename buffer_ref_t>
    requires std::is_same_v<std::remove_cvref_t<buffer_ref_t>, buffer_t>
    [[nodiscard]] static buffer_t hash(std::string_view text, buffer_ref_t&& buffer)
    {
        static_assert(alignof(byte) == alignof(decltype(text)::value_type));
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_sha3_224hash
        WC_CHECK(wc_Sha3_224Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
        return buffer;
    }
};
struct Sha3_256
{
    using buffer_t = std::vector<byte>;
    
    static constexpr std::string_view HASH_NAME = "SHA3-256";
    static constexpr size_t HASH_SIZE = WC_SHA256_DIGEST_SIZE;
    
    template<typename buffer_ref_t>
    requires std::is_same_v<std::remove_cvref_t<buffer_ref_t>, buffer_t>
    [[nodiscard]] static buffer_t hash(std::string_view text, buffer_ref_t&& buffer)
    {
        static_assert(alignof(byte) == alignof(decltype(text)::value_type));
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_sha3_256hash
        WC_CHECK(wc_Sha3_256Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
        return buffer;
    }
};
struct Sha3_384
{
    using buffer_t = std::vector<byte>;
    
    static constexpr std::string_view HASH_NAME = "SHA3-384";
    static constexpr size_t HASH_SIZE = WC_SHA384_DIGEST_SIZE;

    template<typename buffer_ref_t>
    requires std::is_same_v<std::remove_cvref_t<buffer_ref_t>, buffer_t>
    [[nodiscard]] static buffer_t hash(std::string_view text, buffer_ref_t&& buffer)
    {
        static_assert(alignof(byte) == alignof(decltype(text)::value_type));
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_sha3_384hash
        WC_CHECK(wc_Sha3_384Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
        return buffer;
    }
};
struct Sha3_512
{
    using buffer_t = std::vector<byte>;
    
    static constexpr std::string_view HASH_NAME = "SHA3-512";
    static constexpr size_t HASH_SIZE = WC_SHA512_DIGEST_SIZE;
    
    template<typename buffer_ref_t>
    requires std::is_same_v<std::remove_cvref_t<buffer_ref_t>, buffer_t>
    [[nodiscard]] static buffer_t hash(std::string_view text, buffer_ref_t&& buffer)
    {
        static_assert(alignof(byte) == alignof(decltype(text)::value_type));
        // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_sha3_512hash
        WC_CHECK(wc_Sha3_512Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
        return buffer;
    }
};
}   // namespace security