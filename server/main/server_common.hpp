#pragma once
#include "../common/common.hpp"
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
template<typename error_t>
constexpr bool success(error_t errorCode) requires(std::is_same_v<error_t, esp_err_t> || std::is_same_v<error_t, err_t>)
{
	if constexpr (std::is_same_v<error_t, esp_err_t>)
	{
		return errorCode == ESP_OK;
	}

	if constexpr (std::is_same_v<error_t, err_t>)
	{
		return errorCode == ERR_OK;
	}

	// So CPPCHECk stops complaining
	return 0;
}