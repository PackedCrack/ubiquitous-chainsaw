//
// Created by qwerty on 2024-03-01.
//
#pragma once
#include "win32.hpp"
#pragma warning(push)
#pragma warning(disable: 4'265)    // missing virtual destructor - wtf microsfot?
#include <winrt/windows.foundation.h>
#pragma warning(pop)
//
//
//
//
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
}    // namespace sys
