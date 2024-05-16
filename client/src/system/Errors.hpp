#pragma once
#include "../client_defines.hpp"
//
//
//
//
namespace sys
{
enum class ErrorSerialCom
{
    deviceNotFound,
    failedToOpenConnection,
    failedToGetConnectionSettings,
    failedToSetConnectionSettings,
    failedToSetConnectionTimeouts
};
[[nodiscard]] constexpr std::string_view err_serial_com_to_str(ErrorSerialCom err)
{
    // clang-format off
    UNHANDLED_CASE_PROTECTION_ON
    switch (err)
    {
        case ErrorSerialCom::deviceNotFound: return "Device Not Found";
        case ErrorSerialCom::failedToOpenConnection: return "Device";
        case ErrorSerialCom::failedToGetConnectionSettings: return "Failed To Get Connection Settings";
        case ErrorSerialCom::failedToSetConnectionSettings: return "Failed To Set Connection Settings";
        case ErrorSerialCom::failedToSetConnectionTimeouts: return "Failed To Set Connection Timeouts";
    };
    UNHANDLED_CASE_PROTECTION_OFF
    // clang-format on

    std::unreachable();
}
}    // namespace sys
