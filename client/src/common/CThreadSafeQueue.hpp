//
// Created by hackerman on 1/23/24.
//

#pragma once
#include "../defines.hpp"


template<typename element_t>
class CThreadSafeQueue
{
public:
    CThreadSafeQueue()
            : m_List{}
            , m_Lock{}
    {}
    ~CThreadSafeQueue() = default;
    CThreadSafeQueue(const CThreadSafeQueue& other)
            : m_List{}
            , m_Lock{}
    {
        std::lock_guard<std::mutex> lock{ other.m_Lock };
        // cppcheck-suppress useInitializationList
        m_List = other.m_List;  // We cant use initialization list here because we must lock the mutex
    };
    CThreadSafeQueue(CThreadSafeQueue&& other) = default;
    CThreadSafeQueue& operator=(const CThreadSafeQueue& other)
    {
        if(this == &other)
            return *this;
        
        std::lock_guard<std::mutex> lock{ other.m_Lock };
        m_List = other.m_List;
    };
    CThreadSafeQueue& operator=(CThreadSafeQueue&& other) = default;
public:
    template<typename value_t>
    void push(value_t&& val)
    {
        std::lock_guard<std::mutex> lock{ m_Lock };
        
        m_List.push_back(std::forward<value_t>(val));
    }
    template<typename... ctor_args_t>
    void emplace(ctor_args_t&&... arg)
    {
        std::lock_guard<std::mutex> lock{ m_Lock };
        
        m_List.emplace_back(std::forward<ctor_args_t>(arg)...);
    }
    template<typename out_t>
    void pop(out_t* out)
    {
        std::lock_guard<std::mutex> lock{ m_Lock };
        
        *out = std::move(m_List.front());
        m_List.pop_front();
    }
private:
    std::deque<element_t> m_List;
    std::mutex m_Lock;
};