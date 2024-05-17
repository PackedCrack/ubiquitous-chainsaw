//
// Created by qwerty on 2024-04-27.
//
#pragma once
#include "../system/System.hpp"
#include "security/ecc_key.hpp"
// clang-format off


// clang-format on
static constexpr std::string_view CLIENT_PUBLIC_KEY_NAME = "client_public.der";
static constexpr std::string_view CLIENT_PRIVATE_KEY_NAME = "client_private.der";
static constexpr std::string_view SERVER_PUBLIC_KEY_NAME = "server_public.der";
static constexpr std::string_view SERVER_PRIVATE_KEY_NAME = "server_private.der";

[[nodiscard]] std::function<std::expected<std::vector<security::byte>, security::ErrorMakeEccKey>()>
    make_invokable_load_file(std::string_view filename);
template<typename key_t>
requires std::same_as<key_t, security::CEccPublicKey> || std::same_as<key_t, security::CEccPrivateKey>
[[nodiscard]] std::unique_ptr<key_t> load_key(std::string_view keyName)
{
    auto loadKey = make_invokable_load_file(keyName);

    std::optional<key_t> key = security::make_ecc_key<key_t>(loadKey);
    /*if (!key)
    {
        LOG_FATAL_FMT("Cannot continue because \"{}\" could not be loaded.", keyName);
    }*/

    return key ? std::make_unique<key_t>(std::move(*key)) : nullptr;
}
[[nodiscard]] bool keys_exists();
[[nodiscard]] std::function<bool(std::vector<security::byte>&&)> make_invokable_save_file(std::string_view filename);
[[nodiscard]] std::tuple<security::CEccPublicKey, security::CEccPrivateKey> make_ecc_keys();
void save_ecc_keys(security::CEccPublicKey& pub, security::CEccPrivateKey& priv, std::string_view pubkey, std::string_view privkey);
template<typename key_t>
requires std::same_as<key_t, security::CEccPublicKey> || std::same_as<key_t, security::CEccPrivateKey>
void save_ecc_key(key_t& key, std::string_view keyname)
{
    if (!key.write_to_disk(make_invokable_save_file(keyname)))
    {
        LOG_FATAL("Could not save ecc key to disk!");
    }

    if constexpr (std::same_as<key_t, security::CEccPrivateKey>)
    {
        auto restrict_private_key = [keyname](const std::filesystem::path& keyLocation)
        {
            sys::restrict_file_permissions(keyLocation / std::filesystem::path{ keyname });
            return std::expected<std::filesystem::path, std::string>{};
        };
        auto log_failure = [](const std::string& err)
        {
            LOG_ERROR_FMT("Could not set private key to admin owned - "
                          "because filepath to key location could not be retrieved: \"{}\"",
                          err);
            return std::expected<std::filesystem::path, std::string>{};
        };

        [[maybe_unused]] auto result = sys::key_directory().and_then(restrict_private_key).or_else(log_failure);
    }
}
void erase_stored_ecc_keys();
