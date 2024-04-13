//
// Created by qwerty on 2024-02-17.
//

#pragma once
#include "../common/common.hpp"


namespace security
{
// recreating the typedef from wolfcrypt here is the easiest way to not leak WC headers
typedef unsigned char byte;

template<typename hash_t>
concept Hash = requires(hash_t hash)
{
    { hash.size() } -> std::convertible_to<typename decltype(hash)::size_type>;
    { hash.data() } -> std::convertible_to<typename decltype(hash)::const_pointer>;
    { hash.as_string() } -> std::convertible_to<std::string>;
    { hash.as_u8string() } -> std::convertible_to<std::u8string>;
};

template<typename algorithm_t>
concept HashAlgorithm = requires(algorithm_t alg, std::string_view msg, std::vector<byte>& buffer)
{
    { algorithm_t::HASH_NAME } -> std::convertible_to<const std::string_view>;
    { algorithm_t::hash_size() } -> std::convertible_to<size_t>;
    { alg.hash(msg, buffer) } -> std::convertible_to<std::remove_reference_t<decltype(buffer)>>;
};

template<typename algorithm_t>
requires HashAlgorithm<algorithm_t>
class CHash
{
public:
    explicit CHash(std::string_view text)
            : m_Hash{}
    {
        m_Hash.resize(hasher::hash_size());
        create_hash(text);
    };
    ~CHash() = default;
    CHash(const CHash& other) = default;
    CHash(CHash&& other) = default;
    CHash& operator=(const CHash& other) = default;
    CHash& operator=(CHash&& other) = default;
public:
    [[nodiscard]] std::string as_string() const
    {
        return make_str_copy<std::string>();
    }
    [[nodiscard]] std::u8string as_u8string() const
    {
        return make_str_copy<std::u8string>();
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
    void create_hash(std::string_view text)
    {
        m_Hash = hasher::hash(text, m_Hash);
    }
    template<typename string_t>
    [[nodiscard]] string_t make_str_copy() const
    {
        string_t str{};
        str.resize(m_Hash.size());
        ASSERT(str.size() == m_Hash.size(), "Destination and Source buffer size mismatch");
        std::memcpy(str.data(), m_Hash.data(), m_Hash.size());
        
        return str;
    }
private:
    std::vector<byte> m_Hash;
    
    using const_pointer = decltype(m_Hash)::const_pointer;
    using size_type = decltype(m_Hash)::size_type;
    using hasher = algorithm_t;
};
}   // namespace security