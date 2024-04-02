//
// Created by qwerty on 2024-02-17.
//

#pragma once
#include "common.hpp"
// third-party
#define NOMINMAX
#include "wolfcrypt/types.h"    // wolfcrypt brings in windows.h defines? -> this screws up taskflow


namespace security
{
template<typename hash_t>
concept Hash = requires(hash_t hash)
{
    { hash.size() } -> std::convertible_to<typename decltype(hash)::size_type>;
    { hash.data() } -> std::convertible_to<typename decltype(hash)::const_pointer>;
    { hash.as_string() } -> std::convertible_to<std::string>;
    { hash.as_u8string() } -> std::convertible_to<std::u8string>;
};

template<typename algorithm_t, typename buffer_t>
concept HashAlgorithm = requires(algorithm_t alg, std::string_view msg, buffer_t&& hashBuf)
{
    requires common::buffer<buffer_t>;
    { std::is_same_v<typename algorithm_t::buffer_t, buffer_t> };
    
    { algorithm_t::HASH_NAME } -> std::convertible_to<const std::string_view>;
    { algorithm_t::HASH_SIZE } -> std::convertible_to<size_t>;
    { alg.hash(msg, std::forward<buffer_t>(hashBuf)) } -> std::convertible_to<buffer_t>;
};

template<typename algorithm_t>
requires HashAlgorithm<algorithm_t, typename algorithm_t::buffer_t>
class CHash
{
public:
    explicit CHash(std::string_view text)
            : m_Hash{}
    {
        m_Hash.resize(hasher::HASH_SIZE);
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