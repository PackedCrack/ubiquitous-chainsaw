#pragma once
#include "common/defines.hpp"

#ifdef __XTENSA__
    #include "esp_err.h"
    #include "lwip/err.h"
#endif

#ifdef __XTENSA__
    #define LOG_ERROR_ESP(msg, errorCode) logger::log_err(__FILE__, __func__, __LINE__, FMT(msg, esp_err_to_name(errorCode)))

    #define LOG_FATAL_ESP(msg, errorCode) logger::log_fat(__FILE__, __func__, __LINE__, FMT(msg, esp_err_to_name(errorCode)))

    #define LOG_ERROR_LWIP(msg, errorCode) logger::log_err(__FILE__, __func__, __LINE__, FMT(msg, lwip_strerr(errorCode)))

    #define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif
