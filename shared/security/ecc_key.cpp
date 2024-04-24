//
// Created by qwerty on 2024-02-18.
//
#include "ecc_key.hpp"


namespace
{
[[nodiscard]] std::vector<byte> public_key_to_der(ecc_key& key)
{
    std::vector<byte> buffer{};
    buffer.resize(1024u);
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ASN.html#function-wc_eccpublickeytoder
    WCResult result = wc_EccPublicKeyToDer(&key, buffer.data(), common::assert_down_cast<word32>(buffer.size()), TRUE);
    assert(result > 0);
    buffer.resize(static_cast<std::size_t>(result));
    
    return buffer;
}
[[nodiscard]] std::vector<byte> private_key_to_der(ecc_key& key)
{
    std::vector<byte> buffer{};
    buffer.resize(1024u);
    // Can't find doc page for wc_EccPrivateKeyToDer...
    // Could be this: https://www.wolfssl.com/documentation/manuals/wolfssl/group__ASN.html#function-wc_ecckeytoder
    WCResult result = wc_EccPrivateKeyToDer(&key, buffer.data(), common::assert_down_cast<word32>(buffer.size()));
    assert(result > 0);
    buffer.resize(static_cast<std::size_t>(result));
    
    return buffer;
}
}   // namespace

namespace security
{
//////////////////////////////////////
// CEccPublicKey
//////////////////////////////////////
CEccPublicKey::CEccPublicKey(const std::vector<uint8_t>& derData)
        : IEccKey{ derData }
{}
CEccPublicKey& CEccPublicKey::operator=(CEccPublicKey&& other) noexcept
{
    if(this != &other)
        IEccKey::operator=(std::move(other));
    
    return *this;
}
std::vector<byte> CEccPublicKey::to_der()
{
    return public_key_to_der(m_Key);
}
//////////////////////////////////////
// CEccPrivateKey
//////////////////////////////////////
CEccPrivateKey::CEccPrivateKey(const std::vector<uint8_t>& derData)
    : IEccKey{ derData }
{}
CEccPrivateKey& CEccPrivateKey::operator=(CEccPrivateKey&& other) noexcept
{
    if(this != &other)
        IEccKey::operator=(std::move(other));
    
    return *this;
}
std::vector<byte> CEccPrivateKey::to_der()
{
    return private_key_to_der(m_Key);
}
//////////////////////////////////////
// EccKeyPair
//////////////////////////////////////
CEccKeyPair::CEccKeyPair(CRandom& rng)
        : m_Key{}
{
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ECC.html#function-wc_ecc_init
    WC_CHECK(wc_ecc_init(&m_Key));
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ECC.html#function-wc_ecc_make_key
    WC_CHECK(wc_ecc_make_key(rng.wc_struct_p(), 32, &m_Key));   // 32 = 256 bit key
}
CEccKeyPair::~CEccKeyPair()
{
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ECC.html#function-wc_ecc_free
    WC_CHECK(wc_ecc_free(&m_Key));
}
CEccPublicKey CEccKeyPair::public_key()
{
    return CEccPublicKey{ public_key_to_der(m_Key) };
};
CEccPrivateKey CEccKeyPair::private_key()
{
    return CEccPrivateKey{ private_key_to_der(m_Key) };
};
}   // namespace security