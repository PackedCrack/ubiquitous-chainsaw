//
// Created by qwerty on 2024-03-01.
//

#include "SystemAPI.h"
#include <winrt/Windows.Foundation.h>


namespace ble::win
{
SystemAPI::SystemAPI()
{
    winrt::init_apartment();
}
SystemAPI::~SystemAPI()
{
    winrt::uninit_apartment();
}
}   // namespace ble::win
