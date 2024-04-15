#include "gtest/gtest.h"
#include "sha.hpp"
#include "CHash.hpp"


using namespace security;

namespace
{
[[nodiscard]] std::vector<byte> as_binary_data(std::string_view expected)
{
    static std::unordered_map<char, uint8_t> hexAsciiToValue = { { '0', 0 }, { '1', 1 }, { '2', 2 }, { '3', 3 }, { '4', 4 },
                                                                 { '5', 5 }, { '6', 6 }, { '7', 7 }, { '8', 8 }, { '9', 9 },  
                                                                 { 'A', 10 }, { 'B', 11 }, { 'C', 12 }, { 'D', 13 }, { 'E', 14 }, 
                                                                 { 'F', 15 }, { 'a', 10 }, { 'b', 11 }, { 'c', 12 }, { 'd', 13 },
                                                                 { 'e', 14 }, { 'f', 15 } };
    
    std::vector<byte> data{};
    for (size_t i = 0u; i < expected.size(); i = i + 2)
    {
        std::string_view sub = expected.substr(i, 2);
        data.emplace_back(hexAsciiToValue.at(sub[0]));
        uint8_t& value = data.back();
        value = value << 4;
        value = value + hexAsciiToValue.at(sub[1]);
    }

    return data;
}
void hash_data_integrity(const byte* pData, size_t dataSize, std::span<uint8_t> expected)
{
    EXPECT_EQ(dataSize, expected.size());

    std::vector<uint8_t> copiedData{};
    copiedData.resize(dataSize + 1);
    std::memcpy(copiedData.data(), pData, dataSize);

    for (size_t i = 0u; i < expected.size(); ++i)
        EXPECT_EQ(expected[i], copiedData[i]);
}
}   // namespace

/// <summary>
/// Sha 224
/// </summary>
class CHashSha224Test : public ::testing::Test
{
protected:
    CHash<Sha2_224> hash{ "Test string to hash" };
    // https://emn178.github.io/online-tools/sha224.html
    std::string expectedResult = "1a1363206077c19beff06ff526c7a13c07f31c5cb05bf245270dc9d8";
};
TEST_F(CHashSha224Test, Hash_Size)
{
    EXPECT_EQ(hash.size(), Sha2_224::hash_size());
}
TEST_F(CHashSha224Test, Hash_Output_String)
{
    std::string computedHash = hash.as_string();
    EXPECT_EQ(computedHash, expectedResult);
}
TEST_F(CHashSha224Test, Hash_Data_Integrity)
{
    std::vector<byte> expectedHash = as_binary_data(expectedResult);
    hash_data_integrity(hash.data(), hash.size(), expectedHash);
}
/// <summary>
/// Sha 256
/// </summary>
class CHashSha256Test : public ::testing::Test
{
    protected:
    CHash<Sha2_256> hash{ "Test string to hash" };
    // https://emn178.github.io/online-tools/sha256.html
    std::string expectedResult = "0dd8a7fc5978ddc5b751276017d4ffd142a89c67241efd5fcc3ea6d40ec6c743";
};
TEST_F(CHashSha256Test, Hash_Size)
{
    EXPECT_EQ(hash.size(), Sha2_256::hash_size());
}
TEST_F(CHashSha256Test, Hash_Output_String)
{
    std::string computedHash = hash.as_string();
    EXPECT_EQ(computedHash, expectedResult);
}
TEST_F(CHashSha256Test, Hash_Data_Integrity)
{
    std::vector<byte> expectedHash = as_binary_data(expectedResult);
    hash_data_integrity(hash.data(), hash.size(), expectedHash);
}
/// <summary>
/// Sha3_224
/// </summary>
class CHashSha3_224Test : public ::testing::Test
{
    protected:
    CHash<Sha3_224> hash{ "Test string to hash" };
    // https://emn178.github.io/online-tools/sha3_224.html
    std::string expectedResult = "ff6751630fa80e0f63726697b96ec2899bc4db2d4a38d5331709f3c1";
};
TEST_F(CHashSha3_224Test, Hash_Size)
{
    EXPECT_EQ(hash.size(), Sha3_224::hash_size());
}
TEST_F(CHashSha3_224Test, Hash_Output_String)
{
    std::string computedHash = hash.as_string();
    EXPECT_EQ(computedHash, expectedResult);
}
TEST_F(CHashSha3_224Test, Hash_Data_Integrity)
{
    std::vector<byte> expectedHash = as_binary_data(expectedResult);
    hash_data_integrity(hash.data(), hash.size(), expectedHash);
}
/// <summary>
/// Sha3_256
/// </summary>
class CHashSha3_256Test : public ::testing::Test
{
    protected:
    CHash<Sha3_256> hash{ "Test string to hash" };
    // https://emn178.github.io/online-tools/sha3_256.html
    std::string expectedResult = "7959dbab036b6c1f834ce5e5ad9a4743bfb486c4c5c08c61c9bd2c2365070a2a";
};
TEST_F(CHashSha3_256Test, Hash_Size)
{
    EXPECT_EQ(hash.size(), Sha3_256::hash_size());
}
TEST_F(CHashSha3_256Test, Hash_Output_String)
{
    std::string computedHash = hash.as_string();
    EXPECT_EQ(computedHash, expectedResult);
}
TEST_F(CHashSha3_256Test, Hash_Data_Integrity)
{
    std::vector<byte> expectedHash = as_binary_data(expectedResult);
    hash_data_integrity(hash.data(), hash.size(), expectedHash);
}
/// <summary>
/// Sha3_384
/// </summary>
class CHashSha3_384Test : public ::testing::Test
{
    protected:
    CHash<Sha3_384> hash{ "Test string to hash" };
    // https://emn178.github.io/online-tools/sha3_384.html
    std::string expectedResult = "15570e86ffed68a4951262639c1319ac9993916057cb6fe4b80e0b8aea388c1215e049d82279a97cc249cb546ca1f8c3";
};
TEST_F(CHashSha3_384Test, Hash_Size)
{
    EXPECT_EQ(hash.size(), Sha3_384::hash_size());
}
TEST_F(CHashSha3_384Test, Hash_Output_String)
{
    std::string computedHash = hash.as_string();
    EXPECT_EQ(computedHash, expectedResult);
}
TEST_F(CHashSha3_384Test, Hash_Data_Integrity)
{
    std::vector<byte> expectedHash = as_binary_data(expectedResult);
    hash_data_integrity(hash.data(), hash.size(), expectedHash);
}
/// <summary>
/// Sha3_384
/// </summary>
class CHashSha3_512Test : public ::testing::Test
{
    protected:
    CHash<Sha3_512> hash{ "Test string to hash" };
    // https://emn178.github.io/online-tools/sha3_512.html
    std::string expectedResult = "71098856025d177202d0225313a170f459b5a095871cd9ead119c959a731a15f18c250fab39437e2f2005f2dcb20a4df9462feeb16220fae8c125f003cda1ad9";
};
TEST_F(CHashSha3_512Test, Hash_Size)
{
    EXPECT_EQ(hash.size(), Sha3_512::hash_size());
}
TEST_F(CHashSha3_512Test, Hash_Output_String)
{
    std::string computedHash = hash.as_string();
    EXPECT_EQ(computedHash, expectedResult);
}
TEST_F(CHashSha3_512Test, Hash_Data_Integrity)
{
    std::vector<byte> expectedHash = as_binary_data(expectedResult);
    hash_data_integrity(hash.data(), hash.size(), expectedHash);
}