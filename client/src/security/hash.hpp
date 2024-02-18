//
// Created by qwerty on 2024-02-17.
//

#pragma once
#include "defines.hpp"
// third_party
#define NO_OLD_WC_NAMES
#include "wolfcrypt/sha256.h"
#include "wolfssl/wolfcrypt/asn.h"


namespace security
{
template<typename algorithm_t>
concept HashAlgorithm = requires(algorithm_t alg, std::string_view msg, std::vector<byte>& hashBuf)
{
    { algorithm_t::HASH_NAME } -> std::convertible_to<std::string_view>;
    { algorithm_t::HASH_SIZE } -> std::convertible_to<size_t>;
    { alg.hash(msg, hashBuf) } -> std::convertible_to<int32_t>;
};

struct Sha1
{
    static constexpr std::string_view HASH_NAME = "SHA1";
    static constexpr size_t HASH_SIZE = WC_SHA_DIGEST_SIZE;
    [[nodiscard]] static int32_t hash(std::string_view text, std::vector<byte>& buffer);
};
struct Sha256
{
    static constexpr std::string_view HASH_NAME = "SHA256";
    static constexpr size_t HASH_SIZE = WC_SHA256_DIGEST_SIZE;
    [[nodiscard]] static int32_t hash(std::string_view text, std::vector<byte>& buffer);
};
struct Sha512
{
    static constexpr std::string_view HASH_NAME = "SHA512";
    static constexpr size_t HASH_SIZE = WC_SHA512_DIGEST_SIZE;
    [[nodiscard]] static int32_t hash(std::string_view text, std::vector<byte>& buffer);
};


template<typename hash_t>
concept Hash = requires(hash_t hash)
{
    { hash.size() } -> std::convertible_to<typename decltype(hash)::size_type>;
    { hash.data() } -> std::convertible_to<typename decltype(hash)::const_pointer>;
    { hash.as_string() } -> std::convertible_to<std::string>;
    { hash.as_u8string() } -> std::convertible_to<std::u8string>;
};

template<typename algorithm_t> requires HashAlgorithm<algorithm_t>
class CHash
{
private:
    using Result = int32_t;
    enum class ErrorCode : int32_t
    {
        instantiationFailure
    };
public:
    static std::expected<CHash<algorithm_t>, ErrorCode> make_hash(std::string_view text)
    {
        try
        {
            return CHash<algorithm_t>{ text };
        }
        catch(const std::runtime_error& err)
        {
            LOG_ERROR_FMT("Could not instantiate {} Hash. Error Message: \"{}\"", hasher::HASH_NAME, err.what());
            return std::unexpected{ ErrorCode::instantiationFailure };
        }
    }
    ~CHash() = default;
    CHash(const CHash& other) = default;
    CHash(CHash&& other) = default;
    CHash& operator=(const CHash& other) = default;
    CHash& operator=(CHash&& other) = default;
private:
    explicit CHash(std::string_view text)
            : m_Hash{}
    {
        m_Hash.resize(hasher::HASH_SIZE);
        create_hash(text);
    };
public:
    // TODO: should be private?
    void create_hash(std::string_view text)
    {
        static_assert(alignof(byte) == alignof(decltype(text)::value_type));
        Result success = hasher::hash(text, m_Hash);
        if(success != SUCCESS)
            throw std::runtime_error("wolfssl ShaHash convenient function did not return success");
    }
    [[nodiscard]] std::string as_string() const
    {
        return make_str_copy<std::string>(m_Hash);
    }
    [[nodiscard]] std::u8string as_u8string() const
    {
        return make_str_copy<std::u8string>(m_Hash);
    }
    [[nodiscard]] const byte* data() const
    {
        return m_Hash.data();
    }
    [[nodiscard]] size_t size() const
    {
        return m_Hash.size();
    }
private:
    template<typename string_t>
    [[nodiscard]] string_t make_str_copy(const std::vector<byte>& hash) const
    {
        string_t str{};
        str.resize(hash.size());
        ASSERT(str.size() == hash.size(), "Destination and Source buffer size mismatch");
        std::memcpy(str.data(), hash.data(), hash.size());
        
        return str;
    }
private:
    static constexpr Result SUCCESS = 0;
    std::vector<byte> m_Hash;
    
    using const_pointer         = decltype(m_Hash)::const_pointer;
    using size_type         = decltype(m_Hash)::size_type;
    using hasher = algorithm_t;
};
}   // namespace security