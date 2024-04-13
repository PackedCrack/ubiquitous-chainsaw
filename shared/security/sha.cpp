//
// Created by qwerty on 2024-04-13.
//
#include "sha.hpp"
#include "../common/common.hpp"
#include "wc_defines.hpp"
#include "wolfssl/wolfcrypt/asn.h"


namespace security
{
size_t Sha::hash_size()
{
    return WC_SHA_DIGEST_SIZE;
}
std::vector<byte> Sha::hash(std::string_view text, std::vector<byte>& buffer)
{
    static_assert(alignof(byte) == alignof(decltype(text)::value_type));
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_shahash
    WC_CHECK(wc_ShaHash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
    return buffer;
}
/////////////////////////////////////////////////////////////////////////////
size_t Sha2_224::hash_size()
{
    return WC_SHA224_DIGEST_SIZE;
}
std::vector<byte> Sha2_224::hash(std::string_view text, std::vector<byte>& buffer)
{
    static_assert(alignof(byte) == alignof(decltype(text)::value_type));
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_shahash
    WC_CHECK(wc_Sha224Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
    return buffer;
}
/////////////////////////////////////////////////////////////////////////////
size_t Sha2_256::hash_size()
{
    return WC_SHA256_DIGEST_SIZE;
}
std::vector<byte> Sha2_256::hash(std::string_view text, std::vector<byte>& buffer)
{
    static_assert(alignof(byte) == alignof(decltype(text)::value_type));
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_shahash
    WC_CHECK(wc_Sha256Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
    return buffer;
}
/////////////////////////////////////////////////////////////////////////////
size_t Sha2_384::hash_size()
{
    return WC_SHA384_DIGEST_SIZE;
}
std::vector<byte> Sha2_384::hash(std::string_view text, std::vector<byte>& buffer)
{
    static_assert(alignof(byte) == alignof(decltype(text)::value_type));
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_shahash
    WC_CHECK(wc_Sha384Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
    return buffer;
}
/////////////////////////////////////////////////////////////////////////////
size_t Sha2_512::hash_size()
{
    return WC_SHA512_DIGEST_SIZE;
}
std::vector<byte> Sha2_512::hash(std::string_view text, std::vector<byte>& buffer)
{
    static_assert(alignof(byte) == alignof(decltype(text)::value_type));
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_shahash
    WC_CHECK(wc_Sha512Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
    return buffer;
}
/////////////////////////////////////////////////////////////////////////////
size_t Sha3_224::hash_size()
{
    return WC_SHA3_224_DIGEST_SIZE;
}
std::vector<byte> Sha3_224::hash(std::string_view text, std::vector<byte>& buffer)
{
    static_assert(alignof(byte) == alignof(decltype(text)::value_type));
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_shahash
    WC_CHECK(wc_Sha3_224Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
    return buffer;
}
/////////////////////////////////////////////////////////////////////////////
size_t Sha3_256::hash_size()
{
    return WC_SHA3_256_DIGEST_SIZE;
}
std::vector<byte> Sha3_256::hash(std::string_view text, std::vector<byte>& buffer)
{
    static_assert(alignof(byte) == alignof(decltype(text)::value_type));
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_shahash
    WC_CHECK(wc_Sha3_256Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
    return buffer;
}
/////////////////////////////////////////////////////////////////////////////
size_t Sha3_384::hash_size()
{
    return WC_SHA3_384_DIGEST_SIZE;
}
std::vector<byte> Sha3_384::hash(std::string_view text, std::vector<byte>& buffer)
{
    static_assert(alignof(byte) == alignof(decltype(text)::value_type));
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_shahash
    WC_CHECK(wc_Sha3_384Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
    return buffer;
}
/////////////////////////////////////////////////////////////////////////////
size_t Sha3_512::hash_size()
{
    return WC_SHA3_512_DIGEST_SIZE;
}
std::vector<byte> Sha3_512::hash(std::string_view text, std::vector<byte>& buffer)
{
    static_assert(alignof(byte) == alignof(decltype(text)::value_type));
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_shahash
    WC_CHECK(wc_Sha3_512Hash(reinterpret_cast<const byte*>(text.data()), common::assert_down_cast<word32>(text.size()), buffer.data()));
    return buffer;
}
}   // namespace security