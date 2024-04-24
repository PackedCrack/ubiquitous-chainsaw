#include "ble_common.hpp"


namespace ble
{
std::expected<uint16_t, ble::NimbleErrorCode> chr_attri_handle(uint16_t svcUUID, uint16_t chrUUID)
{
    const ble_uuid128_t svcUuid = make_ble_uuid128(svcUUID);
    const ble_uuid128_t chrUuid = make_ble_uuid128(chrUUID);
    uint16_t attributeHandle{};

    NimbleErrorCode result = static_cast<NimbleErrorCode>(ble_gatts_find_chr(reinterpret_cast<const ble_uuid_t*>(&svcUuid), 
                                                         reinterpret_cast<const ble_uuid_t*>(&chrUuid), 
                                                         NULL, &attributeHandle));
    if (result == NimbleErrorCode::success)
    {
        return attributeHandle;
    }
    else
    {
        return std::unexpected(result);
    }
    
}
std::expected<std::string, ble::NimbleErrorCode> current_mac_address(AddressType type)
{
    uint8_t addressType = static_cast<uint8_t>(type);
    std::array<uint8_t, 6u> addr{ 0u };
    auto returnCode = NimbleErrorCode{ ble_hs_id_copy_addr(addressType, addr.data(), nullptr) };
        
    if(returnCode == NimbleErrorCode::success)
    {
        return FMT("{:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}",
                                        addr[5], addr[4], addr[3], 
                                        addr[2], addr[1], addr[0]);
    }
    else if(returnCode == NimbleErrorCode::noConfiguredIdentityAddress)
    {
        LOG_WARN_FMT("Could not retrieve Mac address for type: \"{}\", because no address has been configured for that type.", address_type_to_str(type));
    }
    else if(returnCode == NimbleErrorCode::invalidArguments)
    {
        LOG_ERROR_FMT("Could not retrieve Mac address because the passed adress type parameter: \"{}\" is invalid.", addressType);
    }
    else
    {
        LOG_ERROR_FMT("Unexpected error: \"{}\" - \"{}\", when trying to retrieve MAC address.", 
                        static_cast<int32_t>(returnCode), 
                        nimble_error_to_string(returnCode));
    }

    return std::unexpected( returnCode );
}
} // ble