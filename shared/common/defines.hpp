#pragma once
#include "logger.hpp"
#ifdef __XTENSA__
    #include "esp_err.h"
    #include "lwip/err.h"
#endif

#define NO_FLAGS     0
#define STRINGIFY(x) #x
#define TOSTRING(x)  STRINGIFY(x)

#ifdef _MSC_VER
    #include "intrin.h"
    #define HW_INTERRUPT __debugbreak()
#elif __GNUC__
    #ifdef __XTENSA__
        #define HW_INTERRUPT __asm__("break 0,0")
    #else
        #define HW_INTERRUPT __asm__("int $3")
    #endif
    #define COMPILER_NAME GCC
#elif __clang__
    #define HW_INTERRUPT  __asm__("int 3")    // TODO:: test this..
    #define COMPILER_NAME clang
#endif

#ifdef _MSC_VER
    #define Wswitch                          4'062
    #define UNHANDLED_CASE_PROTECTION_ON     __pragma(warning(error : Wswitch))
    #define UNHANDLED_CASE_PROTECTION_OFF    __pragma(warning(default : Wswitch))
    #define IGNORE_WARNING_PUSH(warningCode) __pragma(warning(push)) __pragma(warning(disable : warningCode))
    #define IGNORE_WARNING_POP               __pragma(warning(pop))
#else
    #define UNHANDLED_CASE_PROTECTION_ON  _Pragma(TOSTRING(COMPILER_NAME diagnostic error "-Wswitch"))
    #define UNHANDLED_CASE_PROTECTION_OFF _Pragma(TOSTRING(COMPILER_NAME diagnostic warning "-Wswitch"))
    #define IGNORE_WARNING(expr, warning)                                                                                                  \
        _Pragma(TOSTRING(COMPILER_NAME diagnostic push)) _Pragma(TOSTRING(COMPILER_NAME diagnostic ignored warning)) expr;                 \
        _Pragma(TOSTRING(COMPILER_NAME diagnostic pop))
#endif

#ifndef NDEBUG
    #define ASSERT(expr, msg)                                                                                                              \
        if (expr)                                                                                                                          \
        {}                                                                                                                                 \
        else                                                                                                                               \
        {                                                                                                                                  \
            LOG_ASSERT(msg);                                                                                                               \
            HW_INTERRUPT;                                                                                                                  \
        }
    #define ASSERT_FMT(expr, msg, ...)                                                                                                     \
        if (expr)                                                                                                                          \
        {}                                                                                                                                 \
        else                                                                                                                               \
        {                                                                                                                                  \
            LOG_ASSERT_FMT(msg, __VA_ARGS__);                                                                                              \
            HW_INTERRUPT;                                                                                                                  \
        }
#else
    #define ASSERT(expr, msg)          expr
    #define ASSERT_FMT(expr, msg, ...) expr
#endif    // !NDEBUG


#define LOG_INFO(msg) logger::log_info(FMT(msg))

#define LOG_INFO_FMT(msg, ...) logger::log_info(FMT(msg, __VA_ARGS__))

#define LOG_WARN(msg) logger::log_warn(__FILE__, __func__, __LINE__, FMT(msg))

#define LOG_WARN_FMT(msg, ...) logger::log_warn(__FILE__, __func__, __LINE__, FMT(msg, __VA_ARGS__))

#define LOG_ERROR(msg) logger::log_err(__FILE__, __func__, __LINE__, FMT(msg))

#define LOG_ERROR_FMT(msg, ...) logger::log_err(__FILE__, __func__, __LINE__, FMT(msg, __VA_ARGS__))

#define LOG_FATAL(msg) logger::log_fat(__FILE__, __func__, __LINE__, FMT(msg))

#define LOG_FATAL_FMT(msg, ...) logger::log_fat(__FILE__, __func__, __LINE__, FMT(msg, __VA_ARGS__))

#define LOG_ASSERT(msg) logger::log_ass(__FILE__, __func__, __LINE__, FMT(msg))

#define LOG_ASSERT_FMT(msg, ...) logger::log_ass(__FILE__, __func__, __LINE__, FMT(msg, __VA_ARGS__))


#ifdef __XTENSA__
    #define LOG_ERROR_ESP(msg, errorCode) logger::log_err(__FILE__, __func__, __LINE__, FMT(msg, esp_err_to_name(errorCode)))

    #define LOG_FATAL_ESP(msg, errorCode) logger::log_fat(__FILE__, __func__, __LINE__, FMT(msg, esp_err_to_name(errorCode)))

    #define LOG_ERROR_LWIP(msg, errorCode) logger::log_err(__FILE__, __func__, __LINE__, FMT(msg, lwip_strerr(errorCode)))

    #define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif
