//
// Created by qwerty on 2024-04-29.
//
#pragma once
#include "../system/System.hpp"
// clang-format off


// clang-format on
namespace common
{
class CCoroutineManager
{
public:
    CCoroutineManager();
    ~CCoroutineManager() = default;
    CCoroutineManager(const CCoroutineManager& other) = delete;
    CCoroutineManager(CCoroutineManager&& other) = delete;
    CCoroutineManager& operator=(const CCoroutineManager& other) = delete;
    CCoroutineManager& operator=(CCoroutineManager&& other) = delete;
public:
    template<typename invokable_t, typename... args_t>
    requires std::is_invocable_r_v<sys::awaitable_t<void>, invokable_t, std::remove_reference_t<args_t>...>
    sys::fire_and_forget_t fire_and_forget(invokable_t&& callable, args_t&&... args)
    {
        m_ActiveCoroutines.fetch_add(1);

        co_await std::invoke(std::forward<invokable_t>(callable), std::forward<args_t>(args)...);

        coroutine_finished();
    }
    void wait_for_all();
private:
    void coroutine_finished();
private:
    std::atomic<int64_t> m_ActiveCoroutines;
    std::mutex m_Mutex;
    std::condition_variable m_ConditionVariable;
};
[[nodiscard]] inline CCoroutineManager& coroutine_manager_instance()
{
    static CCoroutineManager cm{};
    return cm;
}
}    // namespace common
