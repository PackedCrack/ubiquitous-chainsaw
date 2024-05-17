//
// Created by qwerty on 2024-03-01.
//

#include "CSystem.hpp"

// winrt
#include <winrt/windows.foundation.h>
//
//
//
//
namespace sys
{
CSystem::CSystem()
{
    winrt::init_apartment();
}
CSystem::~CSystem()
{
    winrt::uninit_apartment();
}
}    // namespace sys
