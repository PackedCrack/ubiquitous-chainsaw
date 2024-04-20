//
// Created by qwerty on 2024-03-01.
//
#pragma once
#include "../../win32.hpp"
#include <winrt/windows.foundation.h>


namespace sys
{
using os_fire_and_forget_t = winrt::fire_and_forget;
template<typename return_t>
using os_awaitable_t = concurrency::task<return_t>;

class CSystem
{
public:
    CSystem();
    ~CSystem();
    CSystem(const CSystem& other) = delete;
    CSystem(CSystem&& other) = delete;
    CSystem& operator=(const CSystem& other) = delete;
    CSystem& operator=(CSystem&& other) = delete;
};
}   // namespace sys
