//
// Created by qwerty on 2024-04-29.
//
#include "CCoroutineManager.hpp"
// clang-format off


// clang-format on
namespace common
{
CCoroutineManager::CCoroutineManager()
    : m_ActiveCoroutines(0)
{}
void CCoroutineManager::coroutine_finished()
{
    m_ActiveCoroutines.fetch_sub(1);
    ASSERT(m_ActiveCoroutines.load() >= 0, "Expected number of active coroutines to not be negative..");

    if (m_ActiveCoroutines.load() == 0)
    {
        std::lock_guard<std::mutex> lock{ m_Mutex };
        m_ConditionVariable.notify_one();
    }
}
void CCoroutineManager::wait_for_all()
{
#ifndef NDEBUG
    while (m_ActiveCoroutines.load() != 0)
    {
        LOG_INFO_FMT("ACTIVE COROUTINES: {}", m_ActiveCoroutines.load());
    }
#else
    std::unique_lock<std::mutex> lock{ m_Mutex };
    m_ConditionVariable.wait(lock, [this] { return m_ActiveCoroutines.load() == 0; });
#endif
}
}    // namespace common
