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
        , m_CondVar{}
    {}
    ~CThreadSafeQueue() = default;
    CThreadSafeQueue(const CThreadSafeQueue& other) = delete;
    CThreadSafeQueue(CThreadSafeQueue&& other) = default;
    CThreadSafeQueue& operator=(const CThreadSafeQueue& other) = delete;
    CThreadSafeQueue& operator=(CThreadSafeQueue&& other) = default;
public:
    template<typename value_t>
    void push(value_t&& val)
    {
        std::lock_guard<std::mutex> lock{ m_Lock };
        
        m_List.push_back(std::forward<value_t>(val));
        m_CondVar.notify_one();
    }
    template<typename... ctor_args_t>
    void emplace(ctor_args_t&&... arg)
    {
        std::lock_guard<std::mutex> lock{ m_Lock };
        
        m_List.emplace_back(std::forward<ctor_args_t>(arg)...);
        m_CondVar.notify_one();
    }
    template<typename out_t>
    void pop(out_t* out)
    {
        std::unique_lock<std::mutex> lock{ m_Lock };
        auto waitCond = [this](){ return !m_List.empty(); };
        m_CondVar.wait(lock, waitCond);
        
        *out = std::move(m_List.front());
        m_List.pop_front();
    }
private:
    std::deque<element_t> m_List;
    std::mutex m_Lock;
    std::condition_variable m_CondVar;
};