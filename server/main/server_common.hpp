#pragma once
#include "defines.hpp"
// std
#include <string>
#include <optional>
// esp
#include "esp_err.h"


inline std::string esp_err_to_str(esp_err_t code)
{
	std::string err{};
	err.reserve(256u);

	[[maybe_unused]] const char* pStr = esp_err_to_name_r(code, err.data(), err.size());
	ASSERT(pStr == err.data(), "Expected err name to be written to string object");

	return err;
}

template<typename value_t, typename error_t>
struct Result
{
	std::optional<value_t> value;
	error_t error;
};