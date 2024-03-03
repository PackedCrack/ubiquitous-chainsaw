//
// Created by qwerty on 2024-03-03.
//

#pragma once


template<typename derived_t, typename mutex_t, typename enable = void>
struct SharedMutex {};

template<typename derived_t, typename mutex_t>
struct SharedMutex<derived_t, mutex_t, std::enable_if_t<std::is_same_v<mutex_t, std::shared_mutex>>>
{
public:
    void lock_shared()
    {
        static_cast<derived_t*>(this)->m_pMutex->lock_shared();
    }
    void unlock_shared()
    {
        static_cast<derived_t*>(this)->m_pMutex->unlock_shared();
    }
    [[nodiscard]] bool try_lock_shared()
    {
        return static_cast<derived_t*>(this)->m_pMutex->try_lock_shared();
    }
};

template<typename mutex_t>
requires std::is_same_v<mutex_t, std::mutex> || std::is_same_v<mutex_t, std::shared_mutex>
class CMutex : public SharedMutex<CMutex<mutex_t>, mutex_t>
{
public:
    using mutex_type = mutex_t;
    
    friend struct SharedMutex<CMutex<mutex_type>, mutex_type>;
public:
    CMutex()
        : m_pMutex{ std::make_shared<mutex_type>() }
    {};
    CMutex(const CMutex& other) = default;
    CMutex(CMutex&& other) = default;
    CMutex& operator=(const CMutex& other) = default;
    CMutex& operator=(CMutex&& other) = default;
public:
    void lock()
    {
        m_pMutex->lock();
    }
    void unlock()
    {
        m_pMutex->unlock();
    }
    [[nodiscard]] bool try_lock()
    {
        return m_pMutex->try_lock();
    }
private:
    std::shared_ptr<mutex_type> m_pMutex;
};