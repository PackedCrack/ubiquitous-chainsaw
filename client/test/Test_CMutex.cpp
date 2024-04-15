//
// Created by qwerty on 2024-03-03.
//
#include <gtest/gtest.h>
#include <mutex>
#include <shared_mutex>
#include <thread>

#include "common/CMutex.hpp"



class SharedMutexTest : public ::testing::Test
{
protected:
    SharedMutexTest()
        : m_SharedMutex{}
        , m_Writers {}
    {}
    ~SharedMutexTest() override = default;
    
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
    
    
    CMutex<std::shared_mutex> m_SharedMutex;
    std::vector<std::thread> m_Writers;
};
TEST_F(SharedMutexTest, Lock_Shared_UnlockShared)
{
    ASSERT_NO_THROW(m_SharedMutex.lock_shared());
    ASSERT_NO_THROW(m_SharedMutex.unlock_shared());
}
TEST_F(SharedMutexTest, Try_Lock_Shared)
{
    ASSERT_TRUE(m_SharedMutex.try_lock_shared());
    m_SharedMutex.unlock_shared();
}
TEST_F(SharedMutexTest, Shared_Lock_Aquire)
{
    std::shared_mutex smut{};
    ASSERT_TRUE(smut.try_lock_shared());
    ASSERT_FALSE(smut.try_lock());
    
    std::atomic<bool> writersDone{ false };
    std::atomic<int32_t> writersWorking{ 0 };
    auto writer_action = [this, &writersWorking, &writersDone]()
    {
        {
            std::shared_lock lock{ m_SharedMutex };
            writersWorking.fetch_add(1);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
        
        writersWorking.fetch_sub(1);
        if(writersWorking.load() == 0)
        {
            bool expected = false;
            if(writersDone.compare_exchange_strong(expected, true))
            {
                writersDone.notify_one();
            }
        }
    };
    
    m_Writers.emplace_back(writer_action);
    m_Writers.emplace_back(writer_action);
    m_Writers.emplace_back(writer_action);
    m_Writers.emplace_back(writer_action);
    
    
    // spin lock to make sure the writer threads have started
    while(writersWorking.load() == 0)
    {}
    
    ASSERT_FALSE(m_SharedMutex.try_lock());
    while(!writersDone.load())  // in case of sporadic wakeup
        writersDone.wait(false);
    
    ASSERT_TRUE(m_SharedMutex.try_lock());
    m_SharedMutex.unlock();
}
TEST_F(SharedMutexTest, Shared_Lock_In_Use)
{
    std::atomic<bool> writersDone{ false };
    std::atomic<int32_t> writersWorking{ 0 };
    auto writer_action = [this, &writersWorking, &writersDone]()
    {
        {
            ASSERT_FALSE(m_SharedMutex.try_lock_shared());
            writersWorking.fetch_add(1);
            std::shared_lock lock{ m_SharedMutex };
            
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
        
        writersWorking.fetch_sub(1);
        if(writersWorking.load() == 0)
        {
            bool expected = false;
            if(writersDone.compare_exchange_strong(expected, true))
            {
                writersDone.notify_one();
            }
        }
    };
    
    
    {
        std::lock_guard lock{ m_SharedMutex };
        
        size_t numWriters = 4;
        for(size_t i = 0u; i < numWriters; ++i)
            m_Writers.emplace_back(writer_action);
        
        // spin lock to make sure the writer threads have started
        while(writersWorking.load() != numWriters)
        {}
    }
    
    
    while(!writersDone.load())  // in case of sporadic wakeup
        writersDone.wait(false);
}


class MutexTest : public ::testing::Test
{
protected:
    CMutex<std::mutex> m_Mutex;
};

TEST_F(MutexTest, Lock_Unlock)
{
    ASSERT_NO_THROW(m_Mutex.lock());
    ASSERT_NO_THROW(m_Mutex.unlock());
}
TEST_F(MutexTest, Try_Lock)
{
    ASSERT_TRUE(m_Mutex.try_lock());
    m_Mutex.unlock();
}
TEST_F(MutexTest, Lock_Guard)
{
    {
        std::lock_guard lock{ m_Mutex };
        ASSERT_FALSE(m_Mutex.try_lock());
    }
    ASSERT_TRUE(m_Mutex.try_lock());
    m_Mutex.unlock();
}
TEST_F(MutexTest, Unique_Lock_Guard)
{
    {
        std::unique_lock lock{ m_Mutex };
        ASSERT_FALSE(m_Mutex.try_lock());
    }
    ASSERT_TRUE(m_Mutex.try_lock());
    m_Mutex.unlock();
}
