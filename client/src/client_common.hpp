//
// Created by qwerty on 2024-04-27.
//
#pragma once
#include "system/System.hpp"
#include "security/ecc_key.hpp"
// clang-format off


// clang-format on
static constexpr std::string_view CLIENT_PUBLIC_KEY_NAME = "client_public.der";
static constexpr std::string_view CLIENT_PRIVATE_KEY_NAME = "client_private.der";
static constexpr std::string_view SERVER_PUBLIC_KEY_NAME = "server_public.der";
static constexpr std::string_view SERVER_PRIVATE_KEY_NAME = "server_private.der";

[[nodiscard]] std::function<std::expected<std::vector<security::byte>, std::string>()> make_invokable_load_file(std::string_view filename);
template<typename key_t>
requires std::same_as<key_t, security::CEccPublicKey> || std::same_as<key_t, security::CEccPrivateKey>
[[nodiscard]] std::unique_ptr<key_t> load_key(std::string_view keyName)
{
    auto loadKey = make_invokable_load_file(keyName);

    std::optional<key_t> key = security::make_ecc_key<key_t>(loadKey);
    if (!key)
    {
        LOG_FATAL_FMT("Cannot continue because \"{}\" could not be loaded.", keyName);
    }

    return std::make_unique<key_t>(std::move(*key));
}
[[nodiscard]] bool keys_exists();
[[nodiscard]] auto make_invokable_save_file(std::string_view filename);
[[nodiscard]] std::tuple<security::CEccPublicKey, security::CEccPrivateKey> make_ecc_keys();
void save_ecc_keys(security::CEccPublicKey& pub, security::CEccPrivateKey& priv, std::string_view pubkey, std::string_view privkey);
