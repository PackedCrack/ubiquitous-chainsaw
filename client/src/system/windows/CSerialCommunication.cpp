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
{
    // get_com_port();
}
}    // namespace sys
