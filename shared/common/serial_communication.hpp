//
// Created by qwerty on 2024-05-14.
//
#pragma once
#include "defines.hpp"
// std
#include <cstdint>
//
//
//
//
namespace common
{
enum class KeyType : uint8_t
{
    clientPublic,
    serverPublic,
    serverPrivate,
    undefined
};
[[nodiscard]] constexpr std::string_view key_type_to_str(KeyType type)
{
    UNHANDLED_CASE_PROTECTION_ON
    // clang-format off
    switch(type)
    {
    case KeyType::clientPublic: return "Client Public";
    case KeyType::serverPublic: return "Server Public";
    case KeyType::serverPrivate: return "Server Private";
    case KeyType::undefined: return "Undefined";
    }
    // clang-format on
    UNHANDLED_CASE_PROTECTION_OFF

    std::unreachable();
}
struct KeyTransferHeader
{
    uint8_t keyType;
    uint8_t keySize;
};
}