//
// Created by qwerty on 2024-02-18.
//
#pragma once
#include "../common/common.hpp"
// third-party
#include "wc_defines.hpp"
#include "wolfssl/wolfcrypt/asn.h"
// clang-format off


// clang-format on
namespace security
{
template<typename buffer_t, typename out_buffer_t, typename invokable_t>
requires common::const_buffer<buffer_t> ||
         common::buffer<buffer_t> && common::buffer<out_buffer_t> && std::is_invocable_r_v<int, invokable_t, const byte*, word32, byte*>
         decltype(auto) _hash(buffer_t&& data, out_buffer_t&& buffer, invokable_t sha)
{
    static_assert(alignof(byte) == alignof(typename std::remove_cvref_t<decltype(data)>::value_type));
    WC_CHECK(sha(reinterpret_cast<const byte*>(data.data()), common::assert_down_cast<word32>(data.size()), buffer.data()));
    return std::forward<out_buffer_t>(buffer);
}
struct Sha
{
    static constexpr std::string_view HASH_NAME = "SHA";
    static constexpr std::size_t HASH_SIZE = WC_SHA_DIGEST_SIZE;
    template<typename buffer_t, typename out_buffer_t>
    requires common::const_buffer<buffer_t> || common::buffer<buffer_t> && common::buffer<out_buffer_t>
                                               [[nodiscard]] static decltype(auto) hash(buffer_t&& data, out_buffer_t&& buffer)
    {
        return std::forward<out_buffer_t>(_hash(std::forward<buffer_t>(data), std::forward<out_buffer_t>(buffer), wc_ShaHash));
    }
};
struct Sha2_224
{
    static constexpr std::string_view HASH_NAME = "SHA2-224";
    static constexpr std::size_t HASH_SIZE = WC_SHA224_DIGEST_SIZE;
    template<typename buffer_t, typename out_buffer_t>
    requires common::const_buffer<buffer_t> || common::buffer<buffer_t> && common::buffer<out_buffer_t>
                                               [[nodiscard]] static decltype(auto) hash(buffer_t&& data, out_buffer_t&& buffer)
    {
        return std::forward<out_buffer_t>(_hash(std::forward<buffer_t>(data), std::forward<out_buffer_t>(buffer), wc_Sha224Hash));
    }
};
struct Sha2_256
{
    static constexpr std::string_view HASH_NAME = "SHA2-256";
    static constexpr std::size_t HASH_SIZE = WC_SHA256_DIGEST_SIZE;
    template<typename buffer_t, typename out_buffer_t>
    requires common::const_buffer<buffer_t> || common::buffer<buffer_t> && common::buffer<out_buffer_t>
                                               [[nodiscard]] static decltype(auto) hash(buffer_t&& data, out_buffer_t&& buffer)
    {
        return std::forward<out_buffer_t>(_hash(std::forward<buffer_t>(data), std::forward<out_buffer_t>(buffer), wc_Sha256Hash));
    }
};
struct Sha2_384
{
    static constexpr std::string_view HASH_NAME = "SHA2-384";
    static constexpr std::size_t HASH_SIZE = WC_SHA384_DIGEST_SIZE;
    template<typename buffer_t, typename out_buffer_t>
    requires common::const_buffer<buffer_t> || common::buffer<buffer_t> && common::buffer<out_buffer_t>
                                               [[nodiscard]] static decltype(auto) hash(buffer_t&& data, out_buffer_t&& buffer)
    {
        return std::forward<out_buffer_t>(_hash(std::forward<buffer_t>(data), std::forward<out_buffer_t>(buffer), wc_Sha384Hash));
    }
};
struct Sha2_512
{
    static constexpr std::string_view HASH_NAME = "SHA2-512";
    static constexpr std::size_t HASH_SIZE = WC_SHA512_DIGEST_SIZE;
    template<typename buffer_t, typename out_buffer_t>
    requires common::const_buffer<buffer_t> || common::buffer<buffer_t> && common::buffer<out_buffer_t>
                                               [[nodiscard]] static decltype(auto) hash(buffer_t&& data, out_buffer_t&& buffer)
    {
        return std::forward<out_buffer_t>(_hash(std::forward<buffer_t>(data), std::forward<out_buffer_t>(buffer), wc_Sha512Hash));
    }
};
struct Sha3_224
{
    static constexpr std::string_view HASH_NAME = "SHA3-224";
    static constexpr std::size_t HASH_SIZE = WC_SHA3_224_DIGEST_SIZE;
    template<typename buffer_t, typename out_buffer_t>
    requires common::const_buffer<buffer_t> || common::buffer<buffer_t> && common::buffer<out_buffer_t>
                                               [[nodiscard]] static decltype(auto) hash(buffer_t&& data, out_buffer_t&& buffer)
    {
        return std::forward<out_buffer_t>(_hash(std::forward<buffer_t>(data), std::forward<out_buffer_t>(buffer), wc_Sha3_224Hash));
    }
};
struct Sha3_256
{
    static constexpr std::string_view HASH_NAME = "SHA3-256";
    static constexpr std::size_t HASH_SIZE = WC_SHA3_256_DIGEST_SIZE;
    template<typename buffer_t, typename out_buffer_t>
    requires common::const_buffer<buffer_t> || common::buffer<buffer_t> && common::buffer<out_buffer_t>
                                               [[nodiscard]] static decltype(auto) hash(buffer_t&& data, out_buffer_t&& buffer)
    {
        return std::forward<out_buffer_t>(_hash(std::forward<buffer_t>(data), std::forward<out_buffer_t>(buffer), wc_Sha3_256Hash));
    }
};
struct Sha3_384
{
    static constexpr std::string_view HASH_NAME = "SHA3-384";
    static constexpr std::size_t HASH_SIZE = WC_SHA3_384_DIGEST_SIZE;
    template<typename buffer_t, typename out_buffer_t>
    requires common::const_buffer<buffer_t> || common::buffer<buffer_t> && common::buffer<out_buffer_t>
                                               [[nodiscard]] static decltype(auto) hash(buffer_t&& data, out_buffer_t&& buffer)
    {
        return std::forward<out_buffer_t>(_hash(std::forward<buffer_t>(data), std::forward<out_buffer_t>(buffer), wc_Sha3_384Hash));
    }
};
struct Sha3_512
{
    static constexpr std::string_view HASH_NAME = "SHA3-512";
    static constexpr std::size_t HASH_SIZE = WC_SHA3_512_DIGEST_SIZE;
    template<typename buffer_t, typename out_buffer_t>
    requires common::const_buffer<buffer_t> || common::buffer<buffer_t> && common::buffer<out_buffer_t>
                                               [[nodiscard]] static decltype(auto) hash(buffer_t&& data, out_buffer_t&& buffer)
    {
        return std::forward<out_buffer_t>(_hash(std::forward<buffer_t>(data), std::forward<out_buffer_t>(buffer), wc_Sha3_512Hash));
    }
};
}    // namespace security
