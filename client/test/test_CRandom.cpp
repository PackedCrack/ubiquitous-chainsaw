#include "gtest/gtest.h"
#include "CRandom.hpp"


namespace
{
using namespace security;

class CRandomTest : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        m_Rng = CRandom::make_rng();
        ASSERT_TRUE(m_Rng);
    }

    std::expected<CRandom, CRandom::Error> m_Rng = std::unexpected(CRandom::Error::construction);
};

TEST_F(CRandomTest, Move_Constructor) 
{
    ASSERT_TRUE(m_Rng);

    WC_RNG* pRng = m_Rng->wc_struct_p();
    void* pHeap = pRng->heap;
    struct DRBG* pDrbg = pRng->drbg;
    
    CRandom rng{ std::move(*m_Rng) };
    EXPECT_TRUE(pRng->drbg == nullptr);
    EXPECT_TRUE(pRng->heap == nullptr);

    EXPECT_TRUE(rng.wc_struct_p()->drbg == pDrbg);
    EXPECT_TRUE(rng.wc_struct_p()->heap == pHeap);
}

TEST_F(CRandomTest, Move_Assignment) 
{
    ASSERT_TRUE(m_Rng);

    WC_RNG* pRng = m_Rng->wc_struct_p();
    void* pHeap = pRng->heap;
    struct DRBG* pDrbg = pRng->drbg;
    
    std::expected<CRandom, CRandom::Error> rng = CRandom::make_rng();
    *rng = std::move(*m_Rng);

    EXPECT_TRUE(pRng->drbg == nullptr);
    EXPECT_TRUE(pRng->heap == nullptr);

    EXPECT_TRUE(rng->wc_struct_p()->drbg == pDrbg);
    EXPECT_TRUE(rng->wc_struct_p()->heap == pHeap);
}

TEST_F(CRandomTest, Generate_Block_64_Success)
{
    ASSERT_TRUE(m_Rng);

    static constexpr size_t SIZE = 64;

    std::expected<std::vector<uint8_t>, CRandom::Error> block = m_Rng->generate_block(SIZE);
    EXPECT_TRUE(block);
    EXPECT_EQ(block->size(), SIZE);
}

TEST_F(CRandomTest, Generate_Block_128_Success) 
{
    ASSERT_TRUE(m_Rng);

    static constexpr size_t SIZE = 128u;

    std::expected<std::vector<uint8_t>, CRandom::Error> block = m_Rng->generate_block(SIZE);
    EXPECT_TRUE(block);
    EXPECT_EQ(block->size(), SIZE);
}

TEST_F(CRandomTest, Generate_Block_256_Success)
{
    ASSERT_TRUE(m_Rng);

    static constexpr size_t SIZE = 256;

    std::expected<std::vector<uint8_t>, CRandom::Error> block = m_Rng->generate_block(SIZE);
    EXPECT_TRUE(block);
    EXPECT_EQ(block->size(), SIZE);
}

TEST_F(CRandomTest, Generate_Block_Failure) 
{
    ASSERT_TRUE(m_Rng);

    static constexpr size_t SIZE = UINT32_MAX;

    std::expected<std::vector<uint8_t>, CRandom::Error> block = m_Rng->generate_block(SIZE);
    EXPECT_FALSE(block);
    EXPECT_TRUE(block.error() == CRandom::Error::blockGeneration);
}

TEST_F(CRandomTest, Get_WC_RNG_Struct) 
{
    ASSERT_TRUE(m_Rng);

    WC_RNG& rngRef = m_Rng->wc_struct();
    WC_RNG* rngPtr = m_Rng->wc_struct_p();
    EXPECT_NE(rngPtr->drbg, nullptr);

    EXPECT_EQ(rngRef.drbg, rngPtr->drbg);
    EXPECT_EQ(rngRef.heap, rngPtr->heap);
    EXPECT_EQ(rngRef.seed.handle, rngPtr->seed.handle);
}
}   // namespace