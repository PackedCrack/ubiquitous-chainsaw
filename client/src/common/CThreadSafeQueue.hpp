//
// Created by hackerman on 1/23/24.
//

#pragma once
#include "../client_defines.hpp"


template<typename element_t>
class CThreadSafeQueue
{
public:
    CThreadSafeQueue()
            : m_List{}
            , m_Lock{ std::make_unique<std::mutex>() }
    {}
    ~CThreadSafeQueue() = default;
    CThreadSafeQueue(const CThreadSafeQueue& other)
            : m_List{}
            , m_Lock{}
    {
        copy(other);
    };
    CThreadSafeQueue(CThreadSafeQueue&& other) = default;
    CThreadSafeQueue& operator=(const CThreadSafeQueue& other)
    {
        if(this != &other)
        {
            copy(other);
        }
        
        return *this;
    };
    CThreadSafeQueue& operator=(CThreadSafeQueue&& other) = default;
    void copy(const CThreadSafeQueue<element_t>& other)
    {
        std::lock_guard<std::mutex> lock{ *other.m_Lock };
        m_List = other.m_List;
        m_Lock = std::make_unique<std::mutex>();
    }
public:
    template<typename value_t>
    requires std::convertible_to<std::remove_reference_t<value_t>, element_t>
    void push(value_t&& val)
    {
        std::lock_guard<std::mutex> lock{ *m_Lock };
        
        m_List.push_back(std::forward<value_t>(val));
    }
    template<typename value_t>
    requires std::same_as<std::remove_reference_t<value_t>, element_t>
    void push(const std::vector<value_t>& values)
    {
        std::lock_guard<std::mutex> lock{ *m_Lock };
        std::copy(std::begin(values), std::end(values), std::back_inserter(m_List));
    }
    template<typename... ctor_args_t>
    void emplace(ctor_args_t&&... arg)
    {
        std::lock_guard<std::mutex> lock{ *m_Lock };
        
        m_List.emplace_back(std::forward<ctor_args_t>(arg)...);
    }
    template<typename out_t>
    requires std::same_as<std::remove_reference_t<out_t>, element_t> &&
             std::is_reference_v<out_t>
    void pop(out_t&& out)
    {
        std::lock_guard<std::mutex> lock{ *m_Lock };
        
        out = std::move(m_List.front());
        m_List.pop_front();
    }
    [[nodiscard]] size_t size()
    {
        std::lock_guard<std::mutex> lock{ *m_Lock };
        return m_List.size();
    }
    [[nodiscard]] bool empty()
    {
        std::lock_guard<std::mutex> lock{ *m_Lock };
        return m_List.empty();
    }
private:
    std::deque<element_t> m_List;
    std::unique_ptr<std::mutex> m_Lock;
};