//
// Created by qwerty on 2024-01-26.
//

#pragma once
#include "CMutex.hpp"


template<typename hashable_key_t, typename element_t>
class CThreadSafeHashMap
{
public:
    CThreadSafeHashMap()
        : m_Container{}
        , m_Mutex{}
    {};
    ~CThreadSafeHashMap() = default;
    CThreadSafeHashMap(const CThreadSafeHashMap& other) = default;
    CThreadSafeHashMap(CThreadSafeHashMap&& other) = default;
    CThreadSafeHashMap& operator=(const CThreadSafeHashMap& other) = default;
    CThreadSafeHashMap& operator=(CThreadSafeHashMap&& other) = default;
public:
    enum class ErrorCode
    {
        elementNotFound
    };
    struct Error
    {
        std::string msg;
        ErrorCode code;
    };
public:
    // write
    template<typename key_t, typename value_t>
    void insert(key_t&& key, value_t&& value)
    {
        std::unique_lock lock{ m_Mutex };
        m_Container[std::forward<key_t>(key)] = std::forward<value_t>(value);
    }
    template<typename key_t, typename... ctor_args_t>
    [[nodiscard]] bool try_emplace(key_t&& key, ctor_args_t&&... args)
    {
        std::unique_lock lock{ m_Mutex };
        // cppcheck-suppress redundantAssignment
        auto[emplaced, iter] = m_Container.try_emplace(std::forward<key_t>(key), std::forward<ctor_args_t>(args)...);
        
        return emplaced;
    }
    template<typename key_t>
    [[nodiscard]] bool erase(key_t&& key)
    {
        std::unique_lock lock{ m_Mutex };
        size_t numErased = m_Container.erase(std::forward<key_t>(key));
        return numErased != 0u;
    }
    // read
    template<typename key_t>
    [[nodiscard]] std::expected<element_t* const, Error> find(key_t&& key)
    {
        std::shared_lock lock{ m_Mutex };
        auto it = m_Container.find(std::forward<key_t>(key));
        if(it == std::end(m_Container))
            return std::unexpected{ Error{ .msg = "Unable to find element.", .code = ErrorCode::elementNotFound } };
        
        return &(it->second);
    }
    template<typename key_t>
    [[nodiscard]] bool contains(key_t&& key)
    {
        std::shared_lock lock{ m_Mutex };
        return m_Container.contains(std::forward<key_t>(key));
    }
    [[nodiscard]] std::vector<element_t> as_vector()
    {
        // Instead of custom iterators and counting readers we'll just copy the data into a vector and return that..
        std::shared_lock lock{ m_Mutex };
        
        std::vector<element_t> copies{};
        copies.reserve(m_Container.size());
        std::transform(
                std::begin(m_Container),
                std::end(m_Container),
                std::back_inserter(copies),
                [](auto&& pair) { return pair.second; });
        
        //for (const auto& pair : m_Container)
        //    copies.push_back(pair.second);
        
        return copies;
    }
    [[nodiscard]] size_t size()
    {
        std::shared_lock lock{ m_Mutex };
        return m_Container.size();
    }
private:
    std::unordered_map<hashable_key_t, element_t> m_Container;
    CMutex<std::shared_mutex> m_Mutex;
};