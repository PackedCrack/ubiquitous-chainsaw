//
// Created by qwerty on 2024-02-18.
//
#include "CEccKey.hpp"


namespace
{
void invalidate(ecc_key& key)
{
    key.heap = nullptr;
    key.dp = nullptr;
}
bool has_been_moved(ecc_key& key)
{
    return key.heap == nullptr && key.dp == nullptr;
}
[[nodiscard]] std::vector<byte> public_key_to_der(ecc_key& key)
{
    std::vector<byte> buffer{};
    buffer.resize(1024u);
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ASN.html#function-wc_eccpublickeytoder
    WCResult result = wc_EccPublicKeyToDer(&key, buffer.data(), assert_down_cast<word32>(buffer.size()), TRUE);
    assert(result > 0);
    buffer.resize(result);
    
    return buffer;
}
[[nodiscard]] std::vector<byte> private_key_to_der(ecc_key& key)
{
    std::vector<byte> buffer{};
    buffer.resize(1024u);
    // Can't find doc page for wc_EccPrivateKeyToDer...
    WCResult result = wc_EccPrivateKeyToDer(&key, buffer.data(), assert_down_cast<word32>(buffer.size()));
    assert(result > 0);
    buffer.resize(result);
    
    return buffer;
}
}   // namespace

namespace security
{
//////////////////////////////////////
// CEccPublicKey
//////////////////////////////////////
CEccPublicKey::~CEccPublicKey()
{
    if(has_been_moved(m_Key))
        return;
    
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ECC.html#function-wc_ecc_free
    WC_CHECK(wc_ecc_free(&m_Key));
}
CEccPublicKey::CEccPublicKey(const CEccPublicKey& other)
        : m_Key{}
{
    *this = other.copy();
}
CEccPublicKey::CEccPublicKey(CEccPublicKey&& other) noexcept
        : m_Key{ other.m_Key }
{
    static_assert(std::is_trivially_copy_constructible_v<decltype(m_Key)>);
    invalidate(other.m_Key);
}
CEccPublicKey& CEccPublicKey::operator=(const CEccPublicKey& other)
{
    if(this != &other)
    {
        *this = other.copy();
    }
    
    return *this;
}
CEccPublicKey& CEccPublicKey::operator=(CEccPublicKey&& other) noexcept
{
    if(this != &other)
    {
        static_assert(std::is_trivially_copy_constructible_v<decltype(m_Key)>);
        m_Key = other.m_Key;
        invalidate(other.m_Key);
    }
    
    return *this;
}
CEccPublicKey CEccPublicKey::copy() const
{
    ecc_key cpy = m_Key;
    return CEccPublicKey{ public_key_to_der(cpy) };
}
//////////////////////////////////////
// CEccPrivateKey
//////////////////////////////////////
CEccPrivateKey::~CEccPrivateKey()
{
    if(has_been_moved(m_Key))
        return;
    
    // https://www.wolfssl.com/documentation/manuals/wolfssl/group__ECC.html#function-wc_ecc_free
    WC_CHECK(wc_ecc_free(&m_Key));
}
CEccPrivateKey::CEccPrivateKey(const CEccPrivateKey& other)
        : m_Key{}
{
    *this = other.copy();
}
CEccPrivateKey::CEccPrivateKey(CEccPrivateKey&& other) noexcept
        : m_Key{ other.m_Key }
{
    static_assert(std::is_trivially_copy_constructible_v<decltype(m_Key)>);
    invalidate(other.m_Key);
}
CEccPrivateKey& CEccPrivateKey::operator=(const CEccPrivateKey& other)
{
    if(this != &other)
    {
        *this = other.copy();
    }
    
    return *this;
}
CEccPrivateKey& CEccPrivateKey::operator=(CEccPrivateKey&& other) noexcept
{
    if(this != &other)
    {
        static_assert(std::is_trivially_copy_constructible_v<decltype(m_Key)>);
        m_Key = other.m_Key;
        invalidate(other.m_Key);
    }
    
    return *this;
}
CEccPrivateKey CEccPrivateKey::copy() const
{
    ecc_key cpy = m_Key;
    return CEccPrivateKey{ private_key_to_der(cpy) };
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