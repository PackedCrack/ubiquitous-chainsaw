//
// Created by qwerty on 2024-03-01.
//
#pragma once
// win32
#include <windows.h>


namespace sys
{
class System
{
public:
    System();
    ~System();
    System(const System& other) = delete;
    System(System&& other) = delete;
    System& operator=(const System& other) = delete;
    System& operator=(System&& other) = delete;
public:
    [[nodiscard]] bool make_system_tray(HWND hWindow) const;
    [[nodiscard]] bool free_system_tray(HWND hWindow) const;
};
}   // namespace sys
