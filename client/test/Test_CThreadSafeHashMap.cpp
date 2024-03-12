//
// Created by qwerty on 2024-03-02.
//
#include <gtest/gtest.h>
#include "common/CThreadSafeHashMap.hpp"


// First, we define a class that inherits from ::testing::Test
class CThreadSafeHashMapTest : public ::testing::Test {
protected:
    // You may define constructor(s) and destructor(s) as needed
    CThreadSafeHashMapTest()
        : m_pHashMap{ nullptr }
        , m_ReaderThread{}
        , m_WriterThread{}
    {}
    ~CThreadSafeHashMapTest() override = default;
    
    // SetUp() is called immediately before a test starts
    void SetUp() override
    {
        m_pHashMap = std::make_unique<CThreadSafeHashMap<int32_t, int32_t>>();
    }
    // called immediately after a test finishes
    void TearDown() override
    {
        if(m_ReaderThread.joinable())
            m_ReaderThread.join();
        if(m_WriterThread.joinable())
            m_WriterThread.join();
    }
    
    [[nodiscard]] auto insert_elements(size_t numWrites)
    {
        return [this, numWrites]()
        {
            for (size_t i = 0u; i < numWrites; ++i)
            {
                m_pHashMap->insert(i, i);
            }
        };
    }
    [[nodiscard]] std::vector<int32_t> comp_helper_vector(int32_t numElements)
    {
        std::vector<int32_t> vec{};
        for(int32_t i = 0u; i < numElements; ++i)
        {
            vec.emplace_back(i);
        }
        
        return vec;
    }
    
    std::unique_ptr<CThreadSafeHashMap<int32_t, int32_t>> m_pHashMap = nullptr;
    std::thread m_ReaderThread;
    std::thread m_WriterThread;
};


TEST_F(CThreadSafeHashMapTest, AsyncFindAndWrite)
{
    static constexpr size_t NUM_ELEMENTS = 500u;
    std::vector<int32_t> searchableElements = comp_helper_vector(NUM_ELEMENTS / 2u);
    std::reverse(std::begin(searchableElements), std::end(searchableElements));
    
    m_WriterThread = std::thread{ insert_elements(NUM_ELEMENTS) };
    
    
    size_t failsafeCounter = 0u;
    while(true)
    {
        if(searchableElements.empty())
            break;
        
        auto result = m_pHashMap->find(searchableElements.back());
        if(result.has_value())
        {
            EXPECT_TRUE(searchableElements.back() == (*result.value()));
            searchableElements.pop_back();
        }
        EXPECT_TRUE(failsafeCounter++ < NUM_ELEMENTS * 100'000u);
    }
}
TEST_F(CThreadSafeHashMapTest, AsyncWrite)
{
    static constexpr size_t NUM_ELEMENTS = 100u;
    std::shared_mutex mutex{};
    
    auto insertElements = [this](size_t start, size_t numWrites)
    {
        for (size_t i = start; i < start + numWrites; ++i)
        {
            m_pHashMap->insert(i, i);
        }
    };
    std::atomic<int32_t> startVal{ 0 };
    auto writeLogic = [insertElements, &mutex, &startVal]()
    {
        std::shared_lock lock{ mutex };
        
        insertElements(startVal.fetch_add(NUM_ELEMENTS), NUM_ELEMENTS);
    };
    
    
    m_WriterThread = std::thread{ writeLogic };
    std::vector<std::thread> extraWriters{};
    extraWriters.emplace_back(writeLogic);
    extraWriters.emplace_back(writeLogic);
    extraWriters.emplace_back(writeLogic);
    
    
    // hack to make sure the spawned threads are given enough time to start..
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    std::lock_guard lock{ mutex };
    
    
    for(auto&& writer : extraWriters)
    {
        writer.join();
    }
    
    size_t totalThreads = (extraWriters.size() + 1u);
    size_t expectedSize = NUM_ELEMENTS * totalThreads;
    EXPECT_TRUE(m_pHashMap->size() == expectedSize);
    
    std::vector<int32_t> allElements = m_pHashMap->as_vector();
    std::ranges::sort(allElements);
    
    for(size_t i = 0u; i < allElements.size(); ++i)
    {
        EXPECT_TRUE(i == allElements[i]);
    }
}
TEST_F(CThreadSafeHashMapTest, AsyncErase)
{
    static constexpr size_t NUM_ELEMENTS = 1000u;
    static constexpr size_t NUM_WRITER_THREADS = 3u;
    
    std::vector<int32_t> expectedResult = comp_helper_vector(NUM_ELEMENTS * NUM_WRITER_THREADS);
    std::random_device rd{};
    std::mt19937 generator{ rd() };
    std::shuffle(std::begin(expectedResult), std::end(expectedResult), generator);
    
    
    std::ranges::drop_view<std::ranges::ref_view<decltype(expectedResult)>> range
                    = std::views::drop(expectedResult, static_cast<int32_t>(expectedResult.size() * 0.8f));
    std::vector<int32_t> numbersToErase{ std::begin(range), std::end(range) };
    
    for(size_t i = 0u; i < numbersToErase.size(); ++i)
        expectedResult.pop_back();

    std::ranges::sort(expectedResult);
    
    
    
    std::shared_mutex mutex{};
    
    auto insertElements = [this](size_t start, size_t numWrites)
    {
        for (size_t i = start; i < start + numWrites; ++i)
        {
            m_pHashMap->insert(i, i);
        }
    };
    std::atomic<int32_t> startVal{ 0 };
    auto writeLogic = [insertElements, &mutex, &startVal]()
    {
        std::shared_lock lock{ mutex };
        
        insertElements(startVal.fetch_add(NUM_ELEMENTS), NUM_ELEMENTS);
    };
    
    
    std::vector<std::thread> extraWriters{};
    for(size_t i = 0u; i < NUM_WRITER_THREADS; ++i)
        extraWriters.emplace_back(writeLogic);
    
    
    m_WriterThread = std::thread{ [this, &mutex, &numbersToErase]()
    {
        std::shared_lock lock{ mutex };
        
        int32_t successfulErases = 0;
        while(successfulErases < numbersToErase.size())
        {
            for(auto&& number : numbersToErase)
            {
                if(m_pHashMap->erase(number))
                    ++successfulErases;
            }
        }
    }};
    
    // hack to make sure the spawned threads are given enough time to start..
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    std::lock_guard lock{ mutex };
    
    for(auto&& writer : extraWriters)
    {
        writer.join();
    }
    
    
    EXPECT_TRUE(m_pHashMap->size() == expectedResult.size());

    std::vector<int32_t> allElements = m_pHashMap->as_vector();
    std::ranges::sort(allElements);
    EXPECT_TRUE(allElements == expectedResult);
}