//
// Created by qwerty on 2024-02-17.
//
#include "hash.hpp"


namespace security
{
int32_t Sha1::hash(std::string_view text, std::vector<byte>& buffer)
{
    return wc_ShaHash(reinterpret_cast<const byte*>(text.data()), text.size(), buffer.data());
}
int32_t Sha256::hash(std::string_view text, std::vector<byte>& buffer)
{
    return wc_Sha256Hash(reinterpret_cast<const byte*>(text.data()), text.size(), buffer.data());
};
int32_t Sha512::hash(std::string_view text, std::vector<byte>& buffer)
{
    return wc_Sha512Hash(reinterpret_cast<const byte*>(text.data()), text.size(), buffer.data());
};
}   // namespace security