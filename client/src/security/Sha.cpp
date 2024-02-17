//
// Created by qwerty on 2024-02-17.
//
#include "Sha.hpp"
#include "wolfssl/wolfcrypt/asn.h"


namespace security
{
//////////////////////
// CSha1
//////////////////////
std::expected<CSha1, CSha1::ISha::ErrorCode> CSha1::make_sha1(std::string_view text)
{
    try
    {
        return CSha1{ text };
    }
    catch(const std::runtime_error& err)
    {
        LOG_ERROR_FMT("Could not instantiate CSha1: \"{}\"", err.what());
        return std::unexpected{ ErrorCode::instantiationFailure };
    }
}
CSha1::CSha1(std::string_view text)
        : m_Hash{}
{
    m_Hash.resize(SHA_DIGEST_SIZE);
    this->create_hash(text);
}
CSha1::Result CSha1::impl_create_hash(std::string_view text)
{
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_shahash
    return wc_ShaHash(reinterpret_cast<const byte*>(text.data()), text.size(), m_Hash.data());
}
//////////////////////
// CSha256
//////////////////////
std::expected<CSha256, CSha256::ISha::ErrorCode> CSha256::make_sha256(std::string_view text)
{
    try
    {
        return CSha256{ text };
    }
    catch(const std::runtime_error& err)
    {
        LOG_ERROR_FMT("Could not instantiate CSha256: \"{}\"", err.what());
        return std::unexpected{ ErrorCode::instantiationFailure };
    }
}
CSha256::CSha256(std::string_view text)
        : m_Hash{}
{
    m_Hash.resize(SHA512_DIGEST_SIZE);
    this->create_hash(text);
}
CSha256::Result CSha256::impl_create_hash(std::string_view text)
{
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_sha256hash
    return wc_Sha256Hash(reinterpret_cast<const byte*>(text.data()), text.size(), m_Hash.data());
}
//////////////////////
// CSha512
//////////////////////
std::expected<CSha512, CSha512::ISha::ErrorCode> CSha512::make_sha512(std::string_view text)
{
    try
    {
        return CSha512{ text };
    }
    catch(const std::runtime_error& err)
    {
        LOG_ERROR_FMT("Could not instantiate CSha512: \"{}\"", err.what());
        return std::unexpected{ ErrorCode::instantiationFailure };
    }
}
CSha512::CSha512(std::string_view text)
        : m_Hash{}
{
    m_Hash.resize(SHA512_DIGEST_SIZE);
    this->create_hash(text);
}
CSha512::Result CSha512::impl_create_hash(std::string_view text)
{
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__SHA.html#function-wc_sha512hash
    return wc_Sha512Hash(reinterpret_cast<const byte*>(text.data()), text.size(), m_Hash.data());
}
}   // namespace security