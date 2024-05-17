//
// Created by qwerty on 2024-04-27.
//
#include "client_common.hpp"
//
//
//
//
namespace
{
void erase_file(const std::filesystem::path& file)
{
    if (std::filesystem::exists(file))
    {
        ASSERT(std::filesystem::is_regular_file(file), "Expected to remove a file.");
        std::filesystem::remove(file);
    }
}
}    // namespace
std::function<std::expected<std::vector<security::byte>, security::ErrorMakeEccKey>()> make_invokable_load_file(std::string_view filename)
{
    return [filename = std::filesystem::path{ filename }]() -> std::expected<std::vector<security::byte>, security::ErrorMakeEccKey>
    {
        std::expected<std::filesystem::path, std::string> expected = sys::key_directory();
        if (expected)
        {
            std::filesystem::path filepath = expected->string() / filename;
            std::fstream file{ filepath, std::ios::in | std::ios::binary | std::ios::ate };
            if (file.is_open())
            {
                try
                {
                    std::expected<std::vector<security::byte>, security::ErrorMakeEccKey> data{};
                    std::streamsize size = file.tellg();
                    data->resize(size);
                    file.seekg(0);

                    static_assert(alignof(const char) == alignof(decltype(*(data->data()))));
                    file.read(reinterpret_cast<char*>(data->data()), size);

                    return data;
                }
                catch (const std::ios::failure& err)
                {
                    LOG_ERROR_FMT("Exception thrown when trying to read file: \"{}\"", err.what());
                    return std::unexpected(security::ErrorMakeEccKey::fileIOException);
                }
            }
            else
            {
                return std::unexpected(security::ErrorMakeEccKey::couldNotOpenKeyFile);
            }
        }
        else
        {
            return std::unexpected(security::ErrorMakeEccKey::keyLocationNotFound);
        }
    };
}
bool keys_exists()
{
    std::expected<std::filesystem::path, std::string> directory = sys::key_directory();
    if (directory)
    {
        return std::filesystem::exists(*directory / CLIENT_PUBLIC_KEY_NAME) &&
               std::filesystem::exists(*directory / CLIENT_PRIVATE_KEY_NAME) &&
               std::filesystem::exists(*directory / SERVER_PUBLIC_KEY_NAME);
    }

    return false;
}
auto make_invokable_save_file(std::string_view filename)
{
    return [filename = std::filesystem::path{ filename }](std::vector<security::byte>&& key)
    {
        std::expected<std::filesystem::path, std::string> expected = sys::key_directory();
        if (expected)
        {
            std::filesystem::path filepath = expected->string() / filename;
            std::fstream file{ filepath, std::ios::out | std::ios::binary | std::ios::trunc };

            static_assert(alignof(const char) == alignof(decltype(*(key.data()))));
            file.write(reinterpret_cast<const char*>(key.data()), key.size());
            return true;
        }
        else
        {
            LOG_ERROR_FMT("Could not retrieve key location. Failed with: \"{}\"", expected.error());
            return false;
        }
    };
}
std::tuple<security::CEccPublicKey, security::CEccPrivateKey> make_ecc_keys()
{
    security::CRandom rng = security::CRandom::make_rng().value();
    security::CEccKeyPair keyPair{ rng };

    return { keyPair.public_key(), keyPair.private_key() };
}
void save_ecc_keys(security::CEccPublicKey& pub, security::CEccPrivateKey& priv, std::string_view pubkey, std::string_view privkey)
{
    if (!pub.write_to_disk(make_invokable_save_file(pubkey)))
    {
        LOG_FATAL("Could not save public key to disk!");
    }
    if (!priv.write_to_disk(make_invokable_save_file(privkey)))
    {
        LOG_FATAL("Could not save private key to disk!");
    }

    auto restrict_private_key = [privkey](const std::filesystem::path& keyLocation)
    {
        sys::restrict_file_permissions(keyLocation / std::filesystem::path{ privkey });
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
void erase_stored_ecc_keys()
{
    std::expected<std::filesystem::path, std::string> directory = sys::key_directory();
    if (directory)
    {
        erase_file(*directory / CLIENT_PUBLIC_KEY_NAME);
        erase_file(*directory / CLIENT_PRIVATE_KEY_NAME);
        erase_file(*directory / SERVER_PUBLIC_KEY_NAME);
        erase_file(*directory / SERVER_PRIVATE_KEY_NAME);    // shouldnt be stored but check if it is
    }
}
