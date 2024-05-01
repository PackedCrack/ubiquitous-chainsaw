//
// Created by qwerty on 2024-02-17.
//
#pragma once
#include "../common/common.hpp"
#include <cstring>
// clang-format off


// clang-format on
namespace security
{
// recreating the typedef from wolfcrypt here is the easiest way to not leak WC headers
typedef unsigned char byte;

template<typename hash_t>
concept hash = requires(hash_t hash) {
    { hash.size() } -> std::same_as<typename decltype(hash)::size_type>;
    { hash.data() } -> std::same_as<typename decltype(hash)::const_pointer>;
    { hash.as_string() } -> std::same_as<std::string>;
};

template<typename algorithm_t>
concept hash_algorithm = requires(algorithm_t alg, std::array<uint8_t, 26>&& data, std::vector<byte>&& buffer) {
    { algorithm_t::HASH_NAME } -> std::convertible_to<const std::string_view>;
    { algorithm_t::HASH_SIZE } -> std::convertible_to<const std::size_t>;
    //{ alg.hash(std::declval<std::array<uint8_t, 26>>(), std::declval<std::vector<byte>>()) } -> std::same_as<std::vector<byte>>;
};
template<typename algorithm_t>
requires hash_algorithm<algorithm_t>
class CHash
{
public:
    /**
    * @brief Constructs an instance of CHash by creating a hash from the given buffer.
    *
    * This template constructor accepts a buffer of type `buffer_t` and initializes the `m_Hash` member
    * with a hash computed from this buffer.
    *
    * @tparam buffer_t The type of the data buffer, must satisfy `common::const_buffer<buffer_t>`.
    * @param data A universal reference to the data buffer from which to create the hash.
    * @pre The type `buffer_t` must be a model of `common::const_buffer`.
    * @post `m_Hash` is resized to `hash_type::HASH_SIZE` and filled with the hash computed from the provided data.
    */
    template<typename buffer_t>
    requires common::const_buffer<buffer_t> || common::buffer<buffer_t>
    explicit CHash(buffer_t&& data)
        : m_Hash{}
    {
        m_Hash.resize(hash_type::HASH_SIZE);
        create_hash(std::forward<buffer_t>(data));
    }
    /**
    * @brief Constructs an instance of CHash by initializing the hash with values from a given range of a std::span.
    *
    * This constructor receives two constant iterators defining the beginning and the end of a range within
    * a `std::span<uint8_t>` and uses this range to initialize the `m_Hash` member.
    *
    * @param begin A constant iterator to the beginning of the data range within a std::span of uint8_t.
    * @param end A constant iterator to the end of the data range within a std::span of uint8_t.
    * @pre `begin` and `end` must define a valid and non-empty range within the std::span.
    * @post `m_Hash` is initialized with data from the specified range and resized to `hash_type::HASH_SIZE`.
    */
    explicit CHash(std::span<uint8_t>::const_iterator begin, std::span<uint8_t>::const_iterator end)
        : m_Hash{ begin, end }
    {
        // cppcheck-suppress ignoredReturnValue
        ASSERT_FMT(m_Hash.size() == hash_type::HASH_SIZE,
                   "Unexpected size of buffer after inserting span data. Expected: \"{}\", actual size is: \"{}\"",
                   hash_type::HASH_SIZE,
                   m_Hash.size());
        m_Hash.resize(hash_type::HASH_SIZE);
    };
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
                {
                    return false;
                }
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
                {
                    return true;
                }
            }

            return false;
        }

        return true;
    }
public:
    [[nodiscard]] std::string as_string() const
    {
        static constexpr std::array<char, 16u> hex = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

        std::string str{};
        str.reserve(m_Hash.size() * 2u);
        for (auto&& byte_ : m_Hash)
        {
            str.push_back(hex[(byte_ & 0xF0) >> 4]);
            str.push_back(hex[byte_ & 0x0F]);
        }

        return str;
    }
    [[nodiscard]] const byte* data() const { return m_Hash.data(); }
    [[nodiscard]] std::size_t size() const { return m_Hash.size(); }
private:
    template<typename buffer_t>
    requires common::const_buffer<buffer_t> || common::buffer<buffer_t>
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
}    // namespace security
