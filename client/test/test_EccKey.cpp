//
// Created by qwerty on 2024-04-15.
//
#include "gtest/gtest.h"
#include "ecc_key.hpp"
#include "sha.hpp"


using namespace security;


namespace
{
[[nodiscard]] auto mock_make_load_public_from_disk()
{
    return []()
    {
        return std::expected<std::vector<byte>, std::string>{{
            0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a,
            0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x17, 0x15, 0xaf, 0x5e, 0xbb,
            0xb6, 0xd6, 0x76, 0xe3, 0x6b, 0xcc, 0xe5, 0x30, 0xb2, 0xaf, 0xb8, 0xc6, 0x38, 0x09, 0x7e, 0xc1,
            0x8f, 0x29, 0xae, 0x6f, 0x14, 0x9e, 0xfb, 0x38, 0xe8, 0x09, 0x34, 0x8f, 0xc2, 0xed, 0x87, 0xe0,
            0x6d, 0xff, 0x16, 0x0c, 0x5e, 0x4e, 0x5d, 0x75, 0xcc, 0xfa, 0xab, 0x0a, 0x1f, 0xa9, 0x66, 0x22,
            0x45, 0xb4, 0xd6, 0xe5, 0x7f, 0xc5, 0x31, 0xe0, 0xd3, 0xe3, 0xcd
        }};
    };
}
[[nodiscard]] auto mock_make_load_private_from_disk()
{
    return []()
    {
        return std::expected<std::vector<byte>, std::string>{{
            0x30, 0x31, 0x02, 0x01, 0x01, 0x04, 0x20, 0x22, 0x38, 0x5c, 0x68, 0xe1, 0x12, 0x60, 0x1a, 0x03,
            0x14, 0xb3, 0x33, 0xf3, 0xbd, 0x84, 0x23, 0xcb, 0x29, 0x75, 0xcf, 0x1c, 0x8b, 0xf7, 0x57, 0xb6,
            0xc9, 0x1b, 0xbf, 0x62, 0xfc, 0x04, 0xa9, 0xa0, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
            0x03, 0x01, 0x07
        }};
    };
}
template<typename key_t>
void write_to_disk(key_t& key, std::vector<byte>& expected)
{
    std::vector<byte> savedDer{};
    std::function<bool(std::vector<byte>&&)> mockSave { [&savedDer](std::vector<byte>&& data)
                                                        {
                                                            savedDer.append_range(data);
                                                            return true;
                                                        } };
    EXPECT_TRUE(key.write_to_disk(mockSave));
    
    ASSERT_EQ(savedDer.size(), expected.size());
    for(size_t i = 0u; i < savedDer.size(); ++i)
        EXPECT_EQ(savedDer[i], expected[i]);
}
}   // namespace

class CEccKeyTest : public ::testing::Test
{
protected:
    security::CRandom m_Rng = security::CRandom::make_rng().value();
    security::CEccKeyPair keyPair{ m_Rng };
    
    std::unique_ptr<CEccPublicKey> m_pPubKey = nullptr;
    std::unique_ptr<CEccPrivateKey> m_pPrivKey = nullptr;
    void SetUp() override
    {
        m_pPubKey = std::make_unique<CEccPublicKey>(keyPair.public_key());
        m_pPrivKey = std::make_unique<CEccPrivateKey>(keyPair.private_key());
        
        ASSERT_FALSE(m_pPubKey == nullptr);
        ASSERT_FALSE(m_pPrivKey == nullptr);
    }
};
TEST_F(CEccKeyTest, Sign_String)
{
    std::string_view msg = "Very nice message";
    CHash<Sha2_256> hash{ msg };
    std::vector<byte> signature = m_pPrivKey->sign_hash(m_Rng, hash);
    EXPECT_FALSE(signature.empty());
}
TEST_F(CEccKeyTest, Move_Then_Sign_String)
{
    std::string_view msg = "Very nice message";
    CHash<Sha2_256> hash{ msg };
    CEccPrivateKey key = std::move(*m_pPrivKey);
    std::vector<byte> signature = key.sign_hash(m_Rng, hash);
    EXPECT_FALSE(signature.empty());
}
TEST_F(CEccKeyTest, Verify_Signed_String)
{
    std::string_view msg = "Very nice message";
    CHash<Sha2_256> hash{ msg };
    std::vector<byte> signature = m_pPrivKey->sign_hash(m_Rng, hash);
    EXPECT_TRUE(m_pPubKey->verify_hash(signature, hash));
}
TEST_F(CEccKeyTest, Move_Then_Verify_Signed_String)
{
    std::string_view msg = "Very nice message";
    CHash<Sha2_256> hash{ msg };
    std::vector<byte> signature = m_pPrivKey->sign_hash(m_Rng, hash);
    CEccPublicKey key = std::move(*m_pPubKey);
    EXPECT_TRUE(key.verify_hash(signature, hash));
}
TEST(EccKeyTest, Make_Ecc_Keys)
{
    std::optional<CEccPublicKey> pubKey = make_ecc_key<CEccPublicKey>(mock_make_load_public_from_disk());
    EXPECT_TRUE(pubKey);
    std::optional<CEccPrivateKey> privKey = make_ecc_key<CEccPrivateKey>(mock_make_load_private_from_disk());
    EXPECT_TRUE(privKey);
}
TEST(EccKeyTest, Write_Public_Key_To_Disk)
{
    std::vector<uint8_t> validDer = mock_make_load_public_from_disk()().value();
    std::optional<CEccPublicKey> pubKey = make_ecc_key<CEccPublicKey>(mock_make_load_public_from_disk());
    EXPECT_TRUE(pubKey);
    
    write_to_disk(*pubKey, validDer);
}
TEST(EccKeyTest, Write_Private_Key_To_Disk)
{
    std::vector<uint8_t> validDer = mock_make_load_private_from_disk()().value();
    std::optional<CEccPrivateKey> privKey = make_ecc_key<CEccPrivateKey>(mock_make_load_private_from_disk());
    EXPECT_TRUE(privKey);
    
    write_to_disk(*privKey, validDer);
}