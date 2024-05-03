//
// Created by qwerty on 2024-02-18.
//

#pragma once
#include <cstdint>
#include <string_view>
#include "../common/defines.hpp"
// third-party
#include "wolfcrypt/error-crypt.h"


using WCResult = int32_t;
static constexpr WCResult WC_SUCCESS = 0;
static constexpr WCResult WC_FAILURE = -1;

#define WC_ERR_TO_STR(err) wc_GetErrorString(err)

#ifndef NDEBUG
    #define WC_CHECK(expr)                                                                                                                 \
        if (WCResult code = expr; code == WC_SUCCESS)                                                                                      \
        {}                                                                                                                                 \
        else                                                                                                                               \
        {                                                                                                                                  \
            LOG_ASSERT_FMT("WolfCrypt failure: \"{}\"", WC_ERR_TO_STR(code));                                                              \
        }
#else
    #define WC_CHECK(expr) expr
#endif
