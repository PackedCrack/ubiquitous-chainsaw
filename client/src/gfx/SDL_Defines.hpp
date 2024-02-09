#pragma once
#include "defines.hpp"
// third_party
#include "SDL3/SDL_error.h"


#ifndef NDEBUG
	#define SDL_CHECK(expr, msg) \
					if(expr == 0) \
					{} \
					else \
					{	\
						LOG_ERROR(msg); \
						ASSERT_FMT(expr == 0, "SDL Check failed with {}.", SDL_GetError()); \
					}
	#define SDL_CHECK_FMT(expr, msg, ...) \
					if(expr == 0) \
					{} \
					else \
					{	\
						LOG_ERROR_FMT(msg, __VA_ARGS__); \
						ASSERT_FMT(expr == 0, "SDL Check failed with {}.", SDL_GetError()); \
					}
#else
	#define SDL_CHECK(expr, msg) expr
	#define SDL_CHECK_FMT(expr, msg) expr
#endif