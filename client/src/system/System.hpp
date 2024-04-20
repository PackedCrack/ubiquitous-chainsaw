//
// Created by qwerty on 2024-04-03.
//

#pragma once
#include "common/defines.hpp"
// Taskflow must be included BEFORE windows.h
// cppcheck-suppress unknownMacro
IGNORE_WARNING_PUSH(4456)
#include "taskflow/taskflow.hpp"
IGNORE_WARNING_POP

#ifdef WIN32
    #include "windows/CSystem.hpp"
#else
    #error Only windows is implemented
#endif



namespace sys
{
using fire_and_forget_t = os_fire_and_forget_t;
template<typename return_t>
using awaitable_t = os_awaitable_t<return_t>;

void cowabunga();
void auto_wakeup_timer(std::chrono::seconds&& delay);
void restrict_file_permissions(const std::filesystem::path& file);
[[nodiscard]] std::expected<std::filesystem::path, std::string> application_directory();
[[nodiscard]] std::expected<std::filesystem::path, std::string> key_directory();
[[nodiscard]] inline tf::Executor& executor() { static tf::Executor executor{}; return executor; }
}