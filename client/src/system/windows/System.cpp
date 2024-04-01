//
// Created by qwerty on 2024-03-01.
//

#include "System.h"
#include "defines.hpp"
#include "../../resource.hpp"
// win32
//#include <winuser.h>
#include <windows.h>
//#include <shellapi.h>
// winrt
#include <winrt/windows.foundation.h>


namespace
{
}   // namespace
namespace sys
{
System::System()
{
    winrt::init_apartment();
}
System::~System()
{
    winrt::uninit_apartment();
}
}   // namespace sys
