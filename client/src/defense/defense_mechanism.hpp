//
// Created by qwerty on 2024-02-13.
//
#pragma once


namespace sys
{
struct Error
{
    std::string msg;
};
}   // namespace sys

namespace sys::defense
{
void auto_wakeup_timer(std::chrono::seconds&& delay);
void cowabunga();
}   // sys::defense

namespace sys::files
{
void restrict_file_permissions(const std::filesystem::path& file);
[[nodiscard]] std::expected<std::filesystem::path, Error> key_location();
}   // sys::files