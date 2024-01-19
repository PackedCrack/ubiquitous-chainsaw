#pragma once

#include "logger.hpp"


#ifdef _MSC_VER
#include "intrin.h"
	#define HW_INTERRUPT __debugbreak()
#elif __GNUC__
#define HW_INTERRUPT __asm__("int $3")
#elif __clang__
#define HW_INTERRUPT __asm__("int 3") // TODO:: test this..
#endif

#ifndef NDEBUG
#define ASSERT(expr, msg) \
	if(expr) \
	{} \
	else \
	{	\
		LOG_ASSERT(msg); \
		HW_INTERRUPT;	\
	}
#define ASSERT_FMT(expr, msg, ...) \
	if(expr) \
	{} \
	else \
	{	\
		LOG_ASSERT_FMT(msg, __VA_ARGS__); \
		HW_INTERRUPT;	\
	}
#else
#define ASSERT(expr, msg)
#define ASSERT_FMT(expr, msg, ...)
#endif // !NDEBUG


#define LOG_INFO(msg) \
logger::log_info(FMT(msg)) \

#define LOG_INFO_FMT(msg, ...) \
logger::log_info(FMT(msg, __VA_ARGS__)) \

#define LOG_WARN(msg) \
logger::log_warn(__FILE__, __func__, __LINE__, FMT(msg)) \

#define LOG_WARN_FMT(msg, ...) \
logger::log_warn(__FILE__, __func__, __LINE__, FMT(msg, __VA_ARGS__)) \

#define LOG_ERROR(msg) \
logger::log_err(__FILE__, __func__, __LINE__, FMT(msg)) \

#define LOG_ERROR_FMT(msg, ...) \
logger::log_err(__FILE__, __func__, __LINE__, FMT(msg, __VA_ARGS__)) \

#define LOG_FATAL(msg) \
logger::log_fat(__FILE__, __func__, __LINE__, FMT(msg)) \

#define LOG_FATAL_FMT(msg, ...) \
logger::log_fat(__FILE__, __func__, __LINE__, FMT(msg, __VA_ARGS__)) \

#define LOG_ASSERT(msg) \
logger::log_ass(__FILE__, __func__, __LINE__, FMT(msg)) \

#define LOG_ASSERT_FMT(msg, ...) \
logger::log_ass(__FILE__, __func__, __LINE__, FMT(msg, __VA_ARGS__))
