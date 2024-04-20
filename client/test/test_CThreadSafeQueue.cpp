//
// Created by qwerty on 2024-04-19.
//
#include <gtest/gtest.h>
#include "../src/common/CThreadSafeQueue.hpp"
#include <string>
#include <vector>


class BasicQueueTest : public ::testing::Test
{
protected:
    BasicQueueTest()
        : m_Queue{}
        , m_Writers{}
    {}
    ~BasicQueueTest() override = default;
    
    void SetUp() override
    {
        m_Writers.clear();
    }
    void TearDown() override
    {
        for(auto&& thread : m_Writers)
        {
            if(thread.joinable())
                thread.join();
        }
    }
    
    CThreadSafeQueue<std::string> m_Queue;
    std::vector<std::thread> m_Writers;
};
TEST_F(BasicQueueTest, Push_And_Pop_Single)
{
    m_Queue.push("42");
    EXPECT_EQ(m_Queue.size(), 1);
    std::string out{};
    m_Queue.pop(out);
    ASSERT_EQ(out, "42");
}
TEST_F(BasicQueueTest, Push_And_Pop_Multiple)
{
    m_Queue.push("42");
    EXPECT_EQ(m_Queue.size(), 1);
    m_Queue.push("4");
    m_Queue.push("46");
    EXPECT_EQ(m_Queue.size(), 3);
    
    std::string out{};
    m_Queue.pop(out);
    ASSERT_EQ(out, "42");
    m_Queue.pop(out);
    ASSERT_EQ(out, "4");
}
TEST_F(BasicQueueTest, Emplace_Element_Single)
{
    m_Queue.emplace("100");
    EXPECT_EQ(m_Queue.size(), 1);
    
    std::string out{};
    m_Queue.pop(out);
    ASSERT_EQ(out, "100");
}
TEST_F(BasicQueueTest, Emplace_Element_Multiple)
{
    m_Queue.emplace("100");
    EXPECT_EQ(m_Queue.size(), 1);
    m_Queue.emplace("1002");
    EXPECT_EQ(m_Queue.size(), 2);
    
    std::string out{};
    m_Queue.pop(out);
    ASSERT_EQ(out, "100");
    EXPECT_EQ(m_Queue.size(), 1);
    
    m_Queue.emplace("1502");
    EXPECT_EQ(m_Queue.size(), 2);
    m_Queue.emplace("6502");
    EXPECT_EQ(m_Queue.size(), 3);
    
    m_Queue.pop(out);
    ASSERT_EQ(out, "1002");
    EXPECT_EQ(m_Queue.size(), 2);
}
TEST_F(BasicQueueTest, Push_Vector_Of_Elements)
{
    std::vector<std::string> vec{ "111", "222", "333", "444", "555" };
    m_Queue.push(vec);
    EXPECT_EQ(m_Queue.size(), vec.size());
    
    std::string out{};
    m_Queue.pop(out);
    ASSERT_EQ(out, vec[0]);
    EXPECT_EQ(m_Queue.size(), vec.size() - 1u);
    
    m_Queue.pop(out);
    ASSERT_EQ(out, vec[1]);
    EXPECT_EQ(m_Queue.size(), vec.size() - 2u);
    
    m_Queue.push(vec);
    EXPECT_EQ(m_Queue.size(), vec.size() * 2u - 2u);
    
    m_Queue.pop(out);
    ASSERT_EQ(out, vec[2]);
    EXPECT_EQ(m_Queue.size(), vec.size() * 2u - 3u);
}
class ConcurrentQueueTest : public ::testing::Test
{
protected:
    ConcurrentQueueTest()
        : m_Queue{}
        , m_Writers{}
    {}
    void SetUp() override
    {
        
        m_Writers.clear();
        
        m_Writers.emplace_back([this]()
                          {
                              int32_t numPops{ 0 };
                              while(true)
                              {
                                  if(numPops == m_Vec.size())
                                      break;
                                  
                                  if(m_Queue.size() > 0)
                                  {
                                      std::string out{};
                                      m_Queue.pop(out);
                                      EXPECT_EQ(out, m_Vec[numPops]);
                                      ++numPops;
                                  }
                              }
                          });
    }
    void TearDown() override
    {
        for(auto&& thread : m_Writers)
        {
            if(thread.joinable())
                thread.join();
        }
    }
    CThreadSafeQueue<std::string> m_Queue;
    std::vector<std::thread> m_Writers;
    std::vector<std::string> m_Vec{ "111", "222", "333", "444", "555" };
};
TEST_F(ConcurrentQueueTest, Push_Elements)
{
    for(auto&& str : m_Vec)
        m_Queue.push(str);
}
TEST_F(ConcurrentQueueTest, Push_Vector)
{
    m_Queue.push(m_Vec);
}
TEST_F(ConcurrentQueueTest, Emplace_Element)
{
    std::vector<std::string> vecCopy = m_Vec;
    
    for(auto&& str : vecCopy)
        m_Queue.emplace(std::move(str));
}
//TEST_F(IntQueue, ConcurrentPushAndPop)
//{
//    std::vector<std::thread> threads;
//    int out1, out2;
//
//    // Push two elements in parallel
//    threads.emplace_back([&]{ queue.push(1); });
//    threads.emplace_back([&]{ queue.push(2); });
//
//    // Wait for the push operations to complete
//    for (auto& thread : threads) {
//        thread.join();
//    }
//    threads.clear();
//
//    // Pop two elements in parallel
//    threads.emplace_back([&]{ queue.pop(&out1); });
//    threads.emplace_back([&]{ queue.pop(&out2); });
//
//    // Wait for the pop operations to complete
//    for (auto& thread : threads) {
//        thread.join();
//    }
//
//    // We can't assume the order of out1 and out2 because of concurrency
//    ASSERT_TRUE((out1 == 1 && out2 == 2) || (out1 == 2 && out2 == 1));
//}
