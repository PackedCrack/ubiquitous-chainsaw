#include "ble_common.hpp"


namespace ble
{
[[nodiscard]] ble_uuid128_t make_ble_uuid128(uint16_t uniqueValue)
{
	ble_uuid128_t uuid{};
	uuid.u.type = BLE_UUID_TYPE_128;
  
	static_assert(std::is_trivially_copyable_v<decltype(uuid)>);
  	static_assert(std::is_trivially_copyable_v<decltype(BaseUUID)>);
	static_assert(ARRAY_SIZE(uuid.value) == sizeof(decltype(BaseUUID)));
	// cppcheck-suppress sizeofDivisionMemfunc
	std::memcpy(&(uuid.value[0]), &BaseUUID, ARRAY_SIZE(uuid.value));

  	uuid.value[2] = uniqueValue >> 8u;
  	uuid.value[3] = uniqueValue & 0x00FF;

	return uuid;
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