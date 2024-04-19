//
// Created by qwerty on 2024-02-17.
//

#pragma once
#include "../common/common.hpp"
#include <cstring>


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
};

template<typename algorithm_t>
concept HashAlgorithm = requires(algorithm_t alg, std::array<uint8_t, 26>&& data, std::vector<byte>&& buffer)
{
    { algorithm_t::HASH_NAME } -> std::convertible_to<const std::string_view>;
    { algorithm_t::HASH_SIZE } -> std::convertible_to<size_t>;
    //{ alg.hash(data, buffer) } -> std::convertible_to<decltype(buffer)>;
};

template<typename algorithm_t>
requires HashAlgorithm<algorithm_t>
class CHash
{
public:
    template<typename buffer_t>
    requires common::const_buffer<buffer_t>
    explicit CHash(buffer_t&& data)
        : m_Hash{}
    {
        m_Hash.resize(hash_type::HASH_SIZE);
        create_hash(std::forward<buffer_t>(data));
    };
    //explicit CHash(std::string_view text)
    //        : m_Hash{}
    //{
    //    m_Hash.resize(hasher::hash_size());
    //    create_hash(text);
    //};
    ~CHash() = default;
    CHash(const CHash& other) = default;
    CHash(CHash&& other) = default;
    CHash& operator=(const CHash& other) = default;
    CHash& operator=(CHash&& other) = default;
    [[nodiscard]] friend bool operator==(const CHash& lhs, const CHash& rhs)
    {
        if (lhs.m_Hash.size() == rhs.m_Hash.size())
        {
            for (size_t i = 0u; i < lhs.size(); ++i)
            {
                if (lhs.m_Hash[i] != rhs.m_Hash[i])
                    return false;
            }

            return true;
        }
        
        return false;
    }
    [[nodiscard]] friend bool operator!=(const CHash& lhs, const CHash& rhs)
    {
        if (lhs.m_Hash.size() == rhs.m_Hash.size())
        {
            for (size_t i = 0u; i < lhs.size(); ++i)
            {
                if (lhs.m_Hash[i] != rhs.m_Hash[i])
                    return true;
            }

            return false;
        }

        return true;
    }
public:
    [[nodiscard]] std::string as_string() const
    {
        static constexpr std::array<char, 16u> hex = { '0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

        std::string str{};
        str.reserve(m_Hash.size() * 2u);
        for (auto&& byte : m_Hash)
        {
            str.push_back(hex[(byte & 0xF0) >> 4]);
            str.push_back(hex[byte & 0x0F]);
        }

        return str;
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
    template<typename buffer_t>
    requires common::const_buffer<buffer_t>
    void create_hash(buffer_t&& data)
    {
        m_Hash = hash_type::hash(std::forward<buffer_t>(data), m_Hash);
    }
    template<typename string_t>
    [[nodiscard]] string_t make_str_copy() const
    {
        string_t str{};
        str.resize(m_Hash.size());
        // cppcheck-suppress ignoredReturnValue
        ASSERT(str.size() == m_Hash.size(), "Destination and Source buffer size mismatch");
        std::memcpy(str.data(), m_Hash.data(), m_Hash.size());
        
        return str;
    }
private:
    std::vector<byte> m_Hash;
public:
    using const_pointer = decltype(m_Hash)::const_pointer;
    using size_type = decltype(m_Hash)::size_type;
    using hash_type = algorithm_t;
};
}   // namespace security