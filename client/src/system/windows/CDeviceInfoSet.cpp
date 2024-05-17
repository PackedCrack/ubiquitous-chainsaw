#include "CDeviceInfoSet.hpp"
#include "../../client_defines.hpp"
// win32
#pragma comment(lib, "setupapi.lib")
//
//
//
//
namespace sys
{
CDeviceInfoSet::CDeviceInfoSet(const GUID& guid)
    : m_Guid{ guid }
    , m_pDeviceInfoSet{ get_class_devs() }
{}
CDeviceInfoSet::~CDeviceInfoSet()
{
    if (m_pDeviceInfoSet != INVALID_HANDLE_VALUE)
    {
        // https://learn.microsoft.com/en-us/windows/win32/api/setupapi/nf-setupapi-setupdidestroydeviceinfolist
        if (!SetupDiDestroyDeviceInfoList(m_pDeviceInfoSet))
        {
            sys::CErrorMessage err{ GetLastError() };
            LOG_ERROR_FMT("Failed to destroy Device Info Set - Resource Leaked!. Reason: \"{}\"", err.message());
        }
    }
}
CDeviceInfoSet::CDeviceInfoSet(const CDeviceInfoSet& other)
    : m_Guid{ other.m_Guid }
    , m_pDeviceInfoSet{ get_class_devs() }
{}
CDeviceInfoSet::CDeviceInfoSet(CDeviceInfoSet&& other) noexcept
    : m_Guid{ std::move(other.m_Guid) }
    , m_pDeviceInfoSet{ std::exchange(other.m_pDeviceInfoSet, INVALID_HANDLE_VALUE) }
{}
CDeviceInfoSet& CDeviceInfoSet::operator=(const CDeviceInfoSet& other)
{
    if (this != &other)
    {
        m_Guid = other.m_Guid;
        m_pDeviceInfoSet = get_class_devs();
    }

    return *this;
}
CDeviceInfoSet& CDeviceInfoSet::operator=(CDeviceInfoSet&& other) noexcept
{
    m_Guid = std::move(other.m_Guid);
    m_pDeviceInfoSet = std::exchange(other.m_pDeviceInfoSet, INVALID_HANDLE_VALUE);

    return *this;
}
std::vector<std::string> CDeviceInfoSet::enumerate_device_info(property_type property) const
{
    std::vector<std::string> deviceInfo{};
    auto action = [this, &deviceInfo](auto&& property)
    {
        // https://learn.microsoft.com/en-us/windows/win32/api/setupapi/ns-setupapi-sp_devinfo_data
        SP_DEVINFO_DATA deviceInfoData{};
        deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        DWORD index = 0;
        // https://learn.microsoft.com/en-us/windows/win32/api/setupapi/nf-setupapi-setupdienumdeviceinfo
        while (SetupDiEnumDeviceInfo(m_pDeviceInfoSet, index, &deviceInfoData) == TRUE)
        {
            std::array<BYTE, 512> buffer{};
            DWORD requiredSize{};
            // https://learn.microsoft.com/en-us/windows/win32/api/setupapi/nf-setupapi-setupdigetdeviceregistrypropertya
            WIN_CHECK(SetupDiGetDeviceRegistryProperty(m_pDeviceInfoSet,
                                                       &deviceInfoData,
                                                       std::to_underlying(property),
                                                       nullptr,
                                                       buffer.data(),
                                                       static_cast<DWORD>(buffer.size()),
                                                       &requiredSize));
            ASSERT(requiredSize <= buffer.size(), "Pre allocated buffer size to small.");

            deviceInfo.emplace_back(std::begin(buffer), std::end(buffer));
            ++index;
        }

        DWORD error = GetLastError();
        ASSERT(error == ERROR_NO_MORE_ITEMS, "There was an error when enumerating devices..");
    };

    std::visit(action, property);
    return deviceInfo;
}
[[nodiscard]] HDEVINFO CDeviceInfoSet::get_class_devs() const
{
    HDEVINFO pDeviceInfoSet = nullptr;
    // https://learn.microsoft.com/en-us/windows/win32/api/setupapi/nf-setupapi-setupdigetclassdevsw
    // cppcheck-suppress unknownMacro
    WIN_CHECK(pDeviceInfoSet = SetupDiGetClassDevs(&m_Guid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
              pDeviceInfoSet != INVALID_HANDLE_VALUE);

    return pDeviceInfoSet;
}
}    // namespace sys
