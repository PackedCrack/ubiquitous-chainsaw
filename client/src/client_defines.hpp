//
// Created by qwerty on 2024-04-03.
//

#pragma once
#include "../../shared/common/defines.hpp"


#ifdef WIN32
    #include "system/windows/CErrorMessage.hpp"

    #define WIN_CHECK(expr)                                                                                                                \
        if (expr)                                                                                                                          \
        {}                                                                                                                                 \
        else                                                                                                                               \
        {                                                                                                                                  \
            sys::CErrorMessage err{ GetLastError() };                                                                                      \
            LOG_ASSERT_FMT("\"{}\"", err.message());                                                                                       \
            __debugbreak();                                                                                                                \
        }

    #define WIN_CHECK_HRESULT(expr)                                                                                                        \
        if (HRESULT result = expr; result == S_OK)                                                                                         \
        {}                                                                                                                                 \
        else                                                                                                                               \
        {                                                                                                                                  \
            LOG_ASSERT_FMT("\"{}\" failed with: \"{}\"", __func__, result);                                                                \
            __debugbreak();                                                                                                                \
        }
#endif
