//
// Created by qwerty on 2024-03-01.
//
#pragma once
// win32
#include "../../win32.hpp"


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
};
}   // namespace sys
