//
// Created by qwerty on 2024-02-18.
//
#pragma once


namespace security
{
// recreating the typedef from wolfcrypt here is the easiest way to not leak WC headers
typedef unsigned char byte;

struct Sha
{
    static constexpr std::string_view HASH_NAME = "SHA";
    [[nodiscard]] static size_t hash_size();
    [[nodiscard]] static std::vector<byte> hash(std::string_view text, std::vector<byte>& buffer);
};
struct Sha2_224
{
    static constexpr std::string_view HASH_NAME = "SHA2-224";
    [[nodiscard]] static size_t hash_size();
    [[nodiscard]] static std::vector<byte> hash(std::string_view text, std::vector<byte>& buffer);
};
struct Sha2_256
{
    static constexpr std::string_view HASH_NAME = "SHA2-256";
    [[nodiscard]] static size_t hash_size();
    [[nodiscard]] static std::vector<byte> hash(std::string_view text, std::vector<byte>& buffer);
};
struct Sha2_384
{
    static constexpr std::string_view HASH_NAME = "SHA2-384";
    [[nodiscard]] static size_t hash_size();
    [[nodiscard]] static std::vector<byte> hash(std::string_view text, std::vector<byte>& buffer);
};
struct Sha2_512
{
    static constexpr std::string_view HASH_NAME = "SHA2-512";
    [[nodiscard]] static size_t hash_size();
    [[nodiscard]] static std::vector<byte> hash(std::string_view text, std::vector<byte>& buffer);
};
struct Sha3_224
{
    static constexpr std::string_view HASH_NAME = "SHA3-224";
    [[nodiscard]] static size_t hash_size();
    [[nodiscard]] static std::vector<byte> hash(std::string_view text, std::vector<byte>& buffer);
};
struct Sha3_256
{
    static constexpr std::string_view HASH_NAME = "SHA3-256";
    [[nodiscard]] static size_t hash_size();
    [[nodiscard]] static std::vector<byte> hash(std::string_view text, std::vector<byte>& buffer);
};
struct Sha3_384
{
    static constexpr std::string_view HASH_NAME = "SHA3-384";
    [[nodiscard]] static size_t hash_size();
    [[nodiscard]] static std::vector<byte> hash(std::string_view text, std::vector<byte>& buffer);
};
struct Sha3_512
{
    static constexpr std::string_view HASH_NAME = "SHA3-512";
    [[nodiscard]] static size_t hash_size();
    [[nodiscard]] static std::vector<byte> hash(std::string_view text, std::vector<byte>& buffer);
};
}   // namespace security