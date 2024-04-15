#pragma once
#include "common/common.hpp"
// std
#include <string>
#include <string_view>
#include <optional>
// esp
#include "esp_err.h"
// freertos
#include "freertos/task.h"

constexpr std::string_view NVS_ENC_NAMESPACE = "ENC_STORAGE";
constexpr std::string_view NVS_ENC_PRIV_KEY = "ENC_PRIV";
constexpr std::string_view NVS_ENC_PUB_KEY = "ENC_PUB";
constexpr std::string_view NVS_ENC_CLIENT_KEY = "ENC_CLIENT";

constexpr std::string_view NVS_RSSI_NAMESPACE = "RSSI_STORAGE";
constexpr std::string_view NVS_RSSI_KEY = "RSSI";


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
inline void print_task_info(const char* str)
{
    TaskHandle_t xHandle = xTaskGetHandle(str);
	TaskStatus_t status{};
	vTaskGetInfo(xHandle, &status, true, eInvalid);
	LOG_INFO_FMT("Task \"{}\" info:\nTask Number: {}\nStack base: {:p}\nStack min stack space remaining: {}\n", 
					str == nullptr ? "Caller" : str, status.xTaskNumber, static_cast<void*>(status.pxStackBase), status.usStackHighWaterMark);
};