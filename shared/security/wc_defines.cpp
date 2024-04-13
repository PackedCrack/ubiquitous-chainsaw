//
// Created by qwerty on 2024-04-13.
//
#include "wc_defines.hpp"
// third-party
#include "wolfcrypt/error-crypt.h"

std::string_view wc_err_to_str(WCResult code)
{
    return wc_GetErrorString(code);
}