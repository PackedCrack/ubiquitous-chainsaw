//
// Created by qwerty on 2024-02-17.
//

#pragma once
#include "defines.hpp"
// third_party
#include "wolfcrypt/sha256.h"


namespace security
{
template<typename derived_t>
class ISha
{
protected:
    using Result = int32_t;
    enum class ErrorCode : int32_t
    {
        instantiationFailure
    };
public:
    ISha(const ISha& other) = default;
    ISha(ISha&& other) = default;
    ISha& operator=(const ISha& other) = default;
    ISha& operator=(ISha&& other) = default;
protected:
    ISha() = default;
    virtual ~ISha() = default;
public:
    void create_hash(std::string_view text)
    {
        static_assert(alignof(byte) == alignof(decltype(text)::value_type));
        Result success = static_cast<derived_t*>(this)->impl_create_hash(text);
        if(success != SUCCESS)
            throw std::runtime_error("wolfssl ShaHash convenient function did not return success");
    }
    [[nodiscard]] std::string as_string() const
    {
        return make_str_copy<std::string>(static_cast<const derived_t*>(this)->m_Hash);
    }
    [[nodiscard]] std::u8string as_u8string() const
    {
        return make_str_copy<std::u8string>(static_cast<const derived_t*>(this)->m_Hash);
    }
    [[nodiscard]] const byte* data() const
    {
        return static_cast<const derived_t*>(this)->m_Hash.data();
    }
private:
    [[nodiscard]] virtual Result impl_create_hash(std::string_view text) = 0;
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
};

class CSha1 final : public ISha<CSha1>
{
    friend class ISha;
private:
    explicit CSha1(std::string_view text);
public:
    [[nodiscard]] static std::expected<CSha1, ErrorCode> make_sha1(std::string_view text);
    ~CSha1() override = default;
    CSha1(const CSha1& other) = default;
    CSha1(CSha1&& other) = default;
    CSha1& operator=(const CSha1& other) = default;
    CSha1& operator=(CSha1&& other) = default;
private:
    [[nodiscard]] Result impl_create_hash(std::string_view text) override;
private:
    std::vector<byte> m_Hash;
};

class CSha256 final : public ISha<CSha256>
{
    friend class ISha;
private:
    explicit CSha256(std::string_view text);
public:
    [[nodiscard]] static std::expected<CSha256, ErrorCode> make_sha256(std::string_view text);
    ~CSha256() override = default;
    CSha256(const CSha256& other) = default;
    CSha256(CSha256&& other) = default;
    CSha256& operator=(const CSha256& other) = default;
    CSha256& operator=(CSha256&& other) = default;
private:
    [[nodiscard]] Result impl_create_hash(std::string_view text) override;
private:
    std::vector<byte> m_Hash;
};

class CSha512 final : public ISha<CSha512>
{
    friend class ISha;
private:
    explicit CSha512(std::string_view text);
public:
    [[nodiscard]] static std::expected<CSha512, ErrorCode> make_sha512(std::string_view text);
    ~CSha512() override = default;
    CSha512(const CSha512& other) = default;
    CSha512(CSha512&& other) = default;
    CSha512& operator=(const CSha512& other) = default;
    CSha512& operator=(CSha512&& other) = default;
private:
    [[nodiscard]] Result impl_create_hash(std::string_view text) override;
private:
    std::vector<byte> m_Hash;
};
}   // namespace security