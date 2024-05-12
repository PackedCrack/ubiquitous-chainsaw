//
// Created by qwerty on 2024-05-08.
//
#include "gtest/gtest.h"
#include "CReplayProtector.hpp"
#include "ecc_key.hpp"
//
//
//
//
class CReplayProtectorTest : public ::testing::Test
{
protected:
    CReplayProtectorTest()
        : m_Protector{ std::chrono::seconds(1) }
    {}
    ~CReplayProtectorTest() override = default;

    CReplayProtector m_Protector;
};
TEST_F(CReplayProtectorTest, Generate_Random_Block)
{
    const std::vector<uint8_t>& block = m_Protector.generate_random_block();
    EXPECT_GE(block.size(), 64u);
    EXPECT_LE(block.size(), 96u);
}
TEST_F(CReplayProtectorTest, Expected_Random_Data_Not_In_Cache)
{
    std::vector<uint8_t> someData{ 1, 2, 3, 4 };
    EXPECT_FALSE(m_Protector.expected_random_data(someData));
}
TEST_F(CReplayProtectorTest, Expected_Random_Data_In_Cache)
{
    const std::vector<uint8_t>& block = m_Protector.generate_random_block();
    EXPECT_TRUE(m_Protector.expected_random_data(block));
}
TEST_F(CReplayProtectorTest, Expected_Random_Data_Not_In_Cache_After_Delay)
{
    const std::vector<uint8_t>& block = m_Protector.generate_random_block();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_FALSE(m_Protector.expected_random_data(block));
}
TEST_F(CReplayProtectorTest, Expected_Random_Data_In_Cache_After_Delay)
{
    const std::vector<uint8_t>& block = m_Protector.generate_random_block();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(m_Protector.expected_random_data(block));
}
TEST_F(CReplayProtectorTest, Expected_Random_Data_In_Cache_Once)
{
    const std::vector<uint8_t>& block = m_Protector.generate_random_block();
    EXPECT_TRUE(m_Protector.expected_random_data(block));
    EXPECT_FALSE(m_Protector.expected_random_data(block));
    EXPECT_FALSE(m_Protector.expected_random_data(block));
    EXPECT_FALSE(m_Protector.expected_random_data(block));
}