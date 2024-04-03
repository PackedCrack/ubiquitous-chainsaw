//
// Created by qwerty on 2024-04-03.
//

#pragma once
#ifdef WIN32
    #include "windows/CSystem.hpp"
#else
    #error Only windows is implemented
#endif


namespace sys
{
void cowabunga();
void auto_wakeup_timer(std::chrono::seconds&& delay);
void restrict_file_permissions(const std::filesystem::path& file);
[[nodiscard]] std::expected<std::filesystem::path, std::string> application_directory();
[[nodiscard]] std::expected<std::filesystem::path, std::string> key_directory();
}