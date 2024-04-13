//
// Created by qwerty on 2024-02-19.
//

#pragma once
#include "common.hpp"
#include "defines_wc.hpp"
#include "CRandom.hpp"
// third-party
#include "wolfcrypt/aes.h"


#error "Don't include this file - the implementation is incomplete"


namespace security
{
template<typename mode_t>
concept AesMode = requires(mode_t mode)
{
    { std::is_same_v<const size_t, decltype(mode_t::BLOCK_SIZE)> };
    { std::is_same_v<const std::string_view, decltype(mode_t::BLOCK_NAME)> };
};

struct CBC
{
    static constexpr size_t BLOCK_SIZE = 16u;
    static constexpr std::string_view BLOCK_NAME = "CBC";
};

struct GCM
{
    static constexpr size_t BLOCK_SIZE = 12u;
    static constexpr std::string_view BLOCK_NAME = "GCM";
};

template<typename mode_t>
requires AesMode<mode_t>
class CAesKey
{
    using aes_mode = mode_t;
public:
    template<typename string_t>
    explicit CAesKey(string_t&& passphrase)
        : m_Decryptor{}
        , m_Encryptor{}
        , m_Passphrase{ std::forward<string_t>(passphrase) }
    {
        initialize();
    }
    ~CAesKey()
    {
        if(m_Passphrase.empty())
            return;
        
        wc_AesFree(&m_Decryptor);
        wc_AesFree(&m_Encryptor);
    }
    CAesKey(const CAesKey& other) = default;
    CAesKey(CAesKey&& other) = default;
    CAesKey& operator=(const CAesKey& other) = default;
    CAesKey& operator=(CAesKey&& other) = default;
private:
    auto set_keys()
    {
        return [this](std::vector<uint8_t>& dataBlock) -> std::expected<std::vector<uint8_t>, CRandom::Error>
        {
            ASSERT(valid_key_size(m_Passphrase), "The provided passphrase must be 16, 24 or 32 bytes long!");
            
            static_assert(alignof(byte) == alignof(typename decltype(m_Passphrase)::value_type));
            WC_CHECK(wc_AesSetKey(
                    &m_Decryptor,
                    reinterpret_cast<const byte*>(m_Passphrase.data()),
                    common::assert_down_cast<word32>(m_Passphrase.size()),
                    dataBlock.data(),
                    AES_DECRYPTION));
            WC_CHECK(wc_AesSetKey(
                    &m_Encryptor,
                    reinterpret_cast<const byte*>(m_Passphrase.data()),
                    common::assert_down_cast<word32>(m_Passphrase.size()),
                    dataBlock.data(),
                    AES_ENCRYPTION));
        };
    }
    void initialize()
    {
        WC_CHECK(wc_AesInit(&m_Decryptor, nullptr, INVALID_DEVID));
        WC_CHECK(wc_AesInit(&m_Encryptor, nullptr, INVALID_DEVID));
        
        std::expected<CRandom, CRandom::Error> value = CRandom::make_rng();
        if(value)
        {
            CRandom rng = std::move(*value);
            rng.generate_block(aes_mode::BLOCK_SIZE)
            .and_then([this](const std::vector<uint8_t>& dataBlock) -> std::expected<std::vector<uint8_t>, CRandom::Error>
            {
                ASSERT(valid_key_size(m_Passphrase), "The provided passphrase must be 16, 24 or 32 bytes long!");
                
                static_assert(alignof(byte) == alignof(typename decltype(m_Passphrase)::value_type));
                WC_CHECK(wc_AesSetKey(
                        &m_Decryptor,
                        reinterpret_cast<const byte*>(m_Passphrase.data()),
                        common::assert_down_cast<word32>(m_Passphrase.size()),
                        dataBlock.data(),
                        AES_DECRYPTION));
                WC_CHECK(wc_AesSetKey(
                        &m_Encryptor,
                        reinterpret_cast<const byte*>(m_Passphrase.data()),
                        common::assert_down_cast<word32>(m_Passphrase.size()),
                        dataBlock.data(),
                        AES_ENCRYPTION));
                
                return dataBlock;
            })
            .or_else([](CRandom::Error err) -> std::expected<std::vector<uint8_t>, CRandom::Error>
            {
                ASSERT(false, "Block generation failed!");
                
                return std::unexpected{ err };
            });
        }
        else
        {
            // TODO:: hanlde this better?
            ASSERT(false, "Cant create AES keys because CRandom instance failed to create.");
        }
    }
    [[nodiscard]] bool valid_key_size(std::string_view key)
    {
        return key.size() == 16u || key.size() == 24u || key.size() == 32u;
    }
private:
    Aes m_Decryptor;
    Aes m_Encryptor;
    std::string m_Passphrase;
};
}   // namespace security