#pragma once
// win32
#include "win32.hpp"
#include <setupapi.h>
#include <cfgmgr32.h>
//#include <devguid.h>
//#include <ntddser.h>
//#include <regstr.h>
//
//
//
//
namespace sys
{
class CDeviceInfoSet
{
public:
    enum class Property : DWORD
    {
        address = SPDRP_ADDRESS,
        busNumber = SPDRP_BUSNUMBER,
        busTypeGuid = SPDRP_BUSTYPEGUID,
        characteristics = SPDRP_CHARACTERISTICS,
        spdrpClass = SPDRP_CLASS,
        classGuid = SPDRP_CLASSGUID,
        compatibleIDs = SPDRP_COMPATIBLEIDS,
        configFlags = SPDRP_CONFIGFLAGS,
        powerData = SPDRP_DEVICE_POWER_DATA,
        deviceDescription = SPDRP_DEVICEDESC,
        deviceType = SPDRP_DEVTYPE,
        driver = SPDRP_DRIVER,
        enumeratorName = SPDRP_ENUMERATOR_NAME,
        exclusive = SPDRP_EXCLUSIVE,
        friendlyName = SPDRP_FRIENDLYNAME,
        hardwareID = SPDRP_HARDWAREID,
        installState = SPDRP_INSTALL_STATE,
        legacyBusType = SPDRP_LEGACYBUSTYPE,
        locationInformation = SPDRP_LOCATION_INFORMATION,
        locationPaths = SPDRP_LOCATION_PATHS,
        lowerFilters = SPDRP_LOWERFILTERS,
        mfg = SPDRP_MFG,
        physicalDeviceObjectName = SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
        removalPolicy = SPDRP_REMOVAL_POLICY,
        removalPolicyHardwareDefault = SPDRP_REMOVAL_POLICY_HW_DEFAULT,
        removalPolicyOverride = SPDRP_REMOVAL_POLICY_OVERRIDE,
        security = SPDRP_SECURITY,
        securityDescriptorString = SPDRP_SECURITY_SDS,
        service = SPDRP_SERVICE,
        uiNumber = SPDRP_UI_NUMBER,
        uiNumberDescriptionFormat = SPDRP_UI_NUMBER_DESC_FORMAT,
        upperFilters = SPDRP_UPPERFILTERS
    };
    enum class CapabilitiesFlags : DWORD
    {
        lockSupported = CM_DEVCAP_LOCKSUPPORTED,
        ejectSupported = CM_DEVCAP_EJECTSUPPORTED,
        removeable = CM_DEVCAP_REMOVABLE,
        dockDevice = CM_DEVCAP_DOCKDEVICE,
        uniqueID = CM_DEVCAP_UNIQUEID,
        silentInstall = CM_DEVCAP_SILENTINSTALL,
        rawDeviceOK = CM_DEVCAP_RAWDEVICEOK,
        surpriseRemovalOK = CM_DEVCAP_SURPRISEREMOVALOK,
        hardwareDisabled = CM_DEVCAP_HARDWAREDISABLED,
        nonDynamic = CM_DEVCAP_NONDYNAMIC
    };
    using property_type = std::variant<Property, CapabilitiesFlags>;
public:
    explicit CDeviceInfoSet(const GUID& guid);
    ~CDeviceInfoSet();
    CDeviceInfoSet(const CDeviceInfoSet& other);
    CDeviceInfoSet(CDeviceInfoSet&& other) noexcept;
    CDeviceInfoSet& operator=(const CDeviceInfoSet& other);
    CDeviceInfoSet& operator=(CDeviceInfoSet&& other) noexcept;
public:
    [[nodiscard]] std::vector<std::string> enumerate_device_info(property_type property) const;
private:
    [[nodiscard]] HDEVINFO get_class_devs() const;
public:
    GUID m_Guid;
    HDEVINFO m_pDeviceInfoSet = INVALID_HANDLE_VALUE;
};
}    // namespace sys
