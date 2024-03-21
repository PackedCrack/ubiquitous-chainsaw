#pragma once
#include "../../common/ble_services.hpp"
#include "../server_common.hpp"
// std
#include <string_view>
#include <array>
// nimble
#include "host/ble_uuid.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"


namespace ble
{
// GAP DEFINES
constexpr uint16_t INVALID_HANDLE_ID = 65535u;
constexpr uint8_t INVALID_ADDRESS_TYPE = 255u;
constexpr int MAX_UUID_LEN = 128;

constexpr int SUCCESS = 0;
constexpr int FAIL = 1;
constexpr int PROCEDURE_HAS_FINISHED = 14;
constexpr int UNRECOVERABLE_ERROR = 98;
constexpr int RECOVERABLE_ERROR = 99;

// GATT DEFINES
constexpr int DATA_END = 0;
constexpr int NUM_SERVICES = 1;
constexpr int SERVICE_SIZE = 2; // name?

enum class AddressType : uint8_t
{
	publicMac = BLE_ADDR_PUBLIC,
	randomMac = BLE_ADDR_RANDOM,
	invalid = INVALID_ADDRESS_TYPE
};
/*function should be constexpr but cant be until -std=c++2b, because of the static storage.*/
[[nodiscard]] inline std::string_view address_type_to_str(AddressType type)
{
	UNHANDLED_CASE_PROTECTION_ON
	switch(type)
	{
		case AddressType::publicMac: { static constexpr std::string_view str = "publicMac"; return str; }
		case AddressType::randomMac: { static constexpr std::string_view str = "randomMac"; return str; }
		case AddressType::invalid: { static constexpr std::string_view str = "invalid"; return str; }
	}
	UNHANDLED_CASE_PROTECTION_OFF

	__builtin_unreachable();
}
enum class NimbleErrorCode : int32_t
{
	success = SUCCESS, // (not defined by nimble)
    temporaryFailure = BLE_HS_EAGAIN, // Temporary failure; try again
	inProgressOrCompleted = BLE_HS_EALREADY, // Operation already in progress or completed
    invalidArguments = BLE_HS_EINVAL, // One or more arguments are invalid
    toSmallBuffer = BLE_HS_EMSGSIZE, //The provided buffer is too small
    noEntry = BLE_HS_ENOENT, // No entry matching the specified criteria
    resourceExhaustion = BLE_HS_ENOMEM, // Operation failed due to resource exhaustion
    noConnection = BLE_HS_ENOTCONN, // No open connection with the specified handle
    operationDisabled = BLE_HS_ENOTSUP, // Operation disabled at compile time
    unexpectedCallbackBehavior = BLE_HS_EAPP, // Application callback behaved unexpectedly
    invalidPeerCommand = BLE_HS_EBADDATA, // Command from peer is invalid
    osError = BLE_HS_EOS, // Mynewt OS error
    invalidControllerEvent = BLE_HS_ECONTROLLER, // Event from controller is invalid
    operationTimeOut = BLE_HS_ETIMEOUT, // Operation timed out
    operationCompleted = BLE_HS_EDONE, // Operation completed successfully
    isBusy = BLE_HS_EBUSY, // Operation cannot be performed until procedure completes
    peerRejectedConnectionParam = BLE_HS_EREJECT, // Peer rejected a connection parameter update request
    unexpectedFailure = BLE_HS_EUNKNOWN, // Unexpected failure; catch all
    wrongRole = BLE_HS_EROLE, // Operation requires different role (e.g., central vs. peripheral)
    requestTimeOut = BLE_HS_ETIMEOUT_HCI, // HCI request timed out; controller unresponsiv
    eventMemoryExhaustion = BLE_HS_ENOMEM_EVT, // Controller failed to send event due to memory exhaustion (combined host-controller only)
    noConfiguredIdentityAddress = BLE_HS_ENOADDR, // Operation requires an identity address but none configured
    notSynced = BLE_HS_ENOTSYNCED, // Attempt to use the host before it is synced with controller
    insufficientAuthen = BLE_HS_EAUTHEN, // Insufficient authentication
    insufficientAuthor = BLE_HS_EAUTHOR, // Insufficient authorization
    insufficientEncLvl = BLE_HS_EENCRYPT, // Insufficient encryption level
    insufficientKeySize = BLE_HS_EENCRYPT_KEY_SZ, // Insufficient key size
    storageFull = BLE_HS_ESTORE_CAP, // Storage at capacity
    storageIO = BLE_HS_ESTORE_FAIL, // Storage IO error
    preemptedOperation = BLE_HS_EPREEMPTED, // Operation preempted 
    disabledFeature = BLE_HS_EDISABLED, // FDisabled feature
    operationStalled = BLE_HS_ESTALLED //Operation stalled 
};
/*function should be constexpr but cant be until -std=c++2b, because of the static storage.*/
[[nodiscard]] inline std::string_view nimble_error_to_string(NimbleErrorCode error)
{
    UNHANDLED_CASE_PROTECTION_ON
    switch (error)
    {
        case NimbleErrorCode::success: { static constexpr std::string_view str = "Success"; return str; }
        case NimbleErrorCode::temporaryFailure: { static constexpr std::string_view str = "Temporary failure; try again"; return str; }  
        case NimbleErrorCode::inProgressOrCompleted: { static constexpr std::string_view str = "Operation already in progress or completed"; return str; } 
        case NimbleErrorCode::invalidArguments: { static constexpr std::string_view str = "One or more arguments are invalid"; return str; } 
        case NimbleErrorCode::toSmallBuffer: { static constexpr std::string_view str = "The provided buffer is too small"; return str; } 
        case NimbleErrorCode::noEntry: { static constexpr std::string_view str = "No entry matching the specified criteria"; return str; } 
        case NimbleErrorCode::resourceExhaustion: { static constexpr std::string_view str = "Operation failed due to resource exhaustion"; return str; }  
        case NimbleErrorCode::noConnection: { static constexpr std::string_view str = "No open connection with the specified handle"; return str; } 
        case NimbleErrorCode::operationDisabled: { static constexpr std::string_view str = "Operation disabled at compile time"; return str; } 
        case NimbleErrorCode::unexpectedCallbackBehavior: { static constexpr std::string_view str = "Application callback behaved unexpectedly"; return str; } 
        case NimbleErrorCode::invalidPeerCommand: { static constexpr std::string_view str = "Command from peer is invalid"; return str; } 
        case NimbleErrorCode::osError: { static constexpr std::string_view str = "Mynewt OS error"; return str; }  
        case NimbleErrorCode::invalidControllerEvent: { static constexpr std::string_view str = "Event from controller is invalid"; return str; }  
        case NimbleErrorCode::operationTimeOut: { static constexpr std::string_view str = "Operation timed out"; return str; }  
        case NimbleErrorCode::operationCompleted: { static constexpr std::string_view str = "Operation completed successfully"; return str; }  
        case NimbleErrorCode::isBusy: { static constexpr std::string_view str = "Operation cannot be performed until procedure completes"; return str; }  
        case NimbleErrorCode::peerRejectedConnectionParam: { static constexpr std::string_view str = "Peer rejected a connection parameter update request"; return str; }  
        case NimbleErrorCode::unexpectedFailure: { static constexpr std::string_view str = "Unexpected failure; catch all"; return str; }  
        case NimbleErrorCode::wrongRole: { static constexpr std::string_view str = "Operation requires different role (e.g., central vs. peripheral)"; return str; }  
        case NimbleErrorCode::requestTimeOut: { static constexpr std::string_view str = "HCI request timed out; controller unresponsiv"; return str; }  
        case NimbleErrorCode::eventMemoryExhaustion: { static constexpr std::string_view str = "Controller failed to send event due to memory exhaustion (combined host-controller only)"; return str; }  
        case NimbleErrorCode::noConfiguredIdentityAddress: { static constexpr std::string_view str = "Operation requires an identity address but none configured"; return str; }  
        case NimbleErrorCode::notSynced: { static constexpr std::string_view str = "Attempt to use the host before it is synced with controller"; return str; }  
        case NimbleErrorCode::insufficientAuthen:  { static constexpr std::string_view str = "Insufficient authentication"; return str; } 
        case NimbleErrorCode::insufficientAuthor: { static constexpr std::string_view str = "Insufficient authorization"; return str; }  
        case NimbleErrorCode::insufficientEncLvl: { static constexpr std::string_view str = "Insufficient encryption level"; return str; }
        case NimbleErrorCode::insufficientKeySize: { static constexpr std::string_view str = "Insufficient key size"; return str; }  
        case NimbleErrorCode::storageFull: { static constexpr std::string_view str = "Storage at capacity"; return str; }  
        case NimbleErrorCode::storageIO: { static constexpr std::string_view str = "Storage IO error"; return str; }  
        case NimbleErrorCode::preemptedOperation: { static constexpr std::string_view str = "Operation preempted"; return str; }
        case NimbleErrorCode::disabledFeature: { static constexpr std::string_view str = "FDisabled feature"; return str; }
        case NimbleErrorCode::operationStalled: { static constexpr std::string_view str = "Operation stalled"; return str; }
	}
	UNHANDLED_CASE_PROTECTION_OFF

	__builtin_unreachable();
}
[[nodiscard]] inline ble_uuid128_t make_ble_uuid128(uint16_t uniqueValue)
{
	ble_uuid128_t uuid{};
	uuid.u.type = BLE_UUID_TYPE_128;
  
	static_assert(std::is_trivially_copyable_v<decltype(uuid)>);
  	static_assert(std::is_trivially_copyable_v<decltype(BaseUUID)>);
	static_assert(ARRAY_SIZE(uuid.value) == sizeof(decltype(BaseUUID)));
	std::memcpy(&(uuid.value[0]), &BaseUUID, ARRAY_SIZE(uuid.value));

  	uuid.value[2] = uniqueValue >> 8u;
  	uuid.value[3] = uniqueValue & 0x00FF;

	return uuid;
}
template<typename return_t>
requires std::is_same_v<return_t, std::string> || std::is_same_v<return_t, std::array<uint8_t, 6u>>
[[nodiscard]] Result<return_t, NimbleErrorCode> current_mac_address(AddressType type)
{
	uint8_t addressType = static_cast<uint8_t>(type);
	std::array<uint8_t, 6u> addr{ 0u };
	auto returnCode = NimbleErrorCode{ ble_hs_id_copy_addr(addressType, addr.data(), nullptr) };
	
	Result<return_t, NimbleErrorCode> r{
		.value = std::nullopt,
		.error = returnCode
	};
	if(returnCode == NimbleErrorCode::success)
	{
		if constexpr(std::is_same_v<return_t, std::string>)
		{
			r.value = std::make_optional<std::string>(FMT("{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
                                           addr[5], addr[4], addr[3], 
                                           addr[2], addr[1], addr[0]));
		}
		else if constexpr(std::is_same_v<return_t, std::array<uint8_t, 6u>>)
		{
			r.value = std::make_optional<std::array<uint8_t, 6u>>(addr);
		}
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

	return r;
}
}	  // namespace ble