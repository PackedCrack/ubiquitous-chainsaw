#include "CSerialCommunication.hpp"
#include "../../client_defines.hpp"
#include "CDeviceInfoSet.hpp"
// win32
#include <ntddser.h>
//
//
//
//
namespace
{
constexpr DWORD NO_SHARING = 0;
[[nodiscard]] std::string query_com_port(const sys::CDeviceInfoSet& deviceInfoSet, std::size_t tokenIndex)
{
    using Property = sys::CDeviceInfoSet::Property;
    std::vector<std::string> friendlyNames = deviceInfoSet.enumerate_device_info(Property::friendlyName);
    // cppcheck-suppress ignoredReturnValue
    ASSERT(tokenIndex < friendlyNames.size(), "Index out of bounds..");

    const std::string& tokenName = friendlyNames[tokenIndex];
    std::string::size_type begin = tokenName.find("COM");
    ASSERT(begin != std::string::npos, "Expected to find (COM#)");

    std::string::size_type end = tokenName.find_last_of(')');
    ASSERT(end != std::string::npos, "Expected to find (COM#)");
    std::size_t charCount = end - begin;

    return tokenName.substr(begin, charCount);
}
[[nodiscard]] bool access_token_is_connected(const sys::CDeviceInfoSet& deviceInfoSet, std::size_t& tokenIndex)
{
    using Property = sys::CDeviceInfoSet::Property;
    std::vector<std::string> hardwareIDs = deviceInfoSet.enumerate_device_info(Property::hardwareID);
    for (std::size_t i = 0u; i < hardwareIDs.size(); ++i)
    {
        auto&& id = hardwareIDs[i];
        if (id.contains("VID_DEAD&PID_1337"))
        {
            tokenIndex = i;
            return true;
        }
    }

    return false;
}
[[nodiscard]] std::optional<std::string> com_port()
{
    sys::CDeviceInfoSet deviceInfoSet{ GUID_DEVINTERFACE_COMPORT };
    std::size_t tokenIndex{};
    if (!access_token_is_connected(deviceInfoSet, tokenIndex))
    {
        return std::nullopt;
    }

    return std::make_optional<std::string>(query_com_port(deviceInfoSet, tokenIndex));
}
}    // namespace
namespace sys
{
CSerialCommunication::make_type CSerialCommunication::make()
{
    try
    {
        std::optional<std::string> comPort = com_port();
        if (comPort)
        {
            return std::expected<CSerialCommunication, ErrorSerialCom>{ CSerialCommunication{ *comPort } };
        }
        else
        {
            return std::unexpected{ ErrorSerialCom::deviceNotFound };
        }
    }
    catch (const ErrorSerialCom& err)
    {
        return std::unexpected{ err };
    }

    std::unreachable();
}
CSerialCommunication::CSerialCommunication(std::string_view comPort)
    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
    : m_Handle{ CreateFile(comPort.data(), GENERIC_READ | GENERIC_WRITE, NO_SHARING, nullptr, OPEN_EXISTING, 0, nullptr) }
{
    if (m_Handle == INVALID_HANDLE_VALUE)
    {
        CErrorMessage err{ GetLastError() };
        LOG_ERROR_FMT("Failed to construct CSerialCommuncation. Reason: \"{}\"", err.message());
        throw ErrorSerialCom::failedToOpenConnection;
    }

    set_serial_settings();
    set_serial_timeouts();
}
CSerialCommunication::~CSerialCommunication()
{
    if (m_Handle != INVALID_HANDLE_VALUE)
    {
        // https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
        WIN_CHECK(CloseHandle(m_Handle));
    }
}
CSerialCommunication::CSerialCommunication(CSerialCommunication&& other) noexcept
    : m_Handle{ std::exchange(other.m_Handle, INVALID_HANDLE_VALUE) }
{}
CSerialCommunication& CSerialCommunication::operator=(CSerialCommunication&& other) noexcept
{
    m_Handle = std::exchange(other.m_Handle, INVALID_HANDLE_VALUE);

    return *this;
}
void CSerialCommunication::set_serial_timeouts() const
{
    COMMTIMEOUTS timeouts{
        .ReadIntervalTimeout = 50,         /* Maximum time between read chars. */
        .ReadTotalTimeoutMultiplier = 10,  /* Multiplier of characters.        */
        .ReadTotalTimeoutConstant = 50,    /* Constant in milliseconds.        */
        .WriteTotalTimeoutMultiplier = 10, /* Multiplier of characters.        */
        .WriteTotalTimeoutConstant = 50,   /* Constant in milliseconds.        */
    };

    // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setcommtimeouts
    if (!SetCommTimeouts(m_Handle, &timeouts))
    {
        CErrorMessage err{ GetLastError() };
        LOG_ERROR_FMT("Failed to update Serial Timeouts. Reason: \"{}\"", err.message());
        throw ErrorSerialCom::failedToSetConnectionTimeouts;
    }
}
void CSerialCommunication::set_serial_settings() const
{
    DCB serialSettings = serial_settings();

    // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setcommstate
    if (!SetCommState(m_Handle, &serialSettings))
    {
        CErrorMessage err{ GetLastError() };
        LOG_ERROR_FMT("Failed to update Serial Settings. Reason: \"{}\"", err.message());
        throw ErrorSerialCom::failedToSetConnectionSettings;
    }
}
DCB CSerialCommunication::serial_settings() const
{
    DCB serialSettings{};
    serialSettings.DCBlength = sizeof(DCB);

    // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getcommstate
    if (GetCommState(m_Handle, &serialSettings))
    {
        serialSettings.BaudRate = CBR_9600;
        serialSettings.ByteSize = 8;
        serialSettings.StopBits = ONESTOPBIT;
        serialSettings.Parity = NOPARITY;

        return serialSettings;
    }
    else
    {
        CErrorMessage err{ GetLastError() };
        LOG_ERROR_FMT("Failed to retrieve Serial Settings. Reason: \"{}\"", err.message());
        throw ErrorSerialCom::failedToGetConnectionSettings;
    }
}
}    // namespace sys
