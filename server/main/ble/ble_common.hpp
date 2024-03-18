#pragma once
#include "../../common/ble_services.hpp"
#include "../../common/defines.hpp"
// nimble
#include "host/ble_uuid.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"


namespace ble
{
// GAP DEFINES
constexpr uint16_t INVALID_HANDLE_ID = 65535u;
constexpr uint8_t INVALID_ADDRESS_TYPE = 255u;
constexpr int RANDOM_BLUETOOTH_ADDRESS = 1;
constexpr int PUBLIC_BLUETOOTH_ADDRESS = 0;
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

enum class ErrorCode : int32_t
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
    operationStalled = BLE_HS_ESTALLED, //Operation stalled 
	unknown = INT32_MAX
};

inline ble_uuid128_t make_ble_uuid128(uint16_t uniqueValue)
{
	ble_uuid128_t uuid{};
	uuid.u.type = BLE_UUID_TYPE_128;
  
	static_assert(std::is_trivially_copyable_v<decltype(uuid)>);
  	static_assert(std::is_trivially_copyable_v<decltype(BaseUID)>);
	static_assert(ARRAY_SIZE(uuid.value) == sizeof(decltype(BaseUID)));
	std::memcpy(&(uuid.value[0]), &BaseUID, ARRAY_SIZE(uuid.value));

  	uuid.value[2] = uniqueValue >> 8u;
  	uuid.value[3] = uniqueValue & 0x00FF;

	return uuid;
}
}  // namespace ble
