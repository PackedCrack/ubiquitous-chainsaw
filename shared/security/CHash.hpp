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
    void create_hash(std::string_view text)
    {
        m_Hash = hasher::hash(text, m_Hash);
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
    using hasher = algorithm_t;
};
}   // namespace security