#pragma once

/* STD */
#include <string>
#include <cstdio>
#include <type_traits>
#include <vector>
#include <array>
#include <optional>
//#include <format> // apparently i dont have c++20

//#include <utility>

/* Project */
#include "defines.hpp"
#include "ServerDefines.hpp"

/* BLE */
#include "host/ble_hs_adv.h"
#include "host/ble_gap.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"


//#include "nimble/nimble_port.h"


namespace ble
{

struct BleClientCharacteristic
{
    ble_uuid_any_t uuid;
    uint16_t handle;
    uint16_t handleValue;
    uint8_t properties;
};

struct BleClientService 
{
    ble_uuid_any_t uuid;
    uint16_t connHandle;
    uint16_t handleStart;
    uint16_t handleEnd;
    std::vector<BleClientCharacteristic> characteristics;
};


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

struct Error
{
	ErrorCode code;
	std::string msg;
};


class CConnectionHandle // NOTE: Will this be needed for GATT services???? probably drop connectino if not authenticated
{

public:
    CConnectionHandle();
    ~CConnectionHandle();
    CConnectionHandle(const CConnectionHandle& other) = delete; // why shouldnt i be able to copy it?, because if i make i copy, the other one can be droped and then the copy is invalid?
    CConnectionHandle(CConnectionHandle&& other) noexcept;
    CConnectionHandle& operator=(const CConnectionHandle& other) = delete;
    CConnectionHandle& operator=(CConnectionHandle&& other);

public:
    [[nodiscard]] uint16_t handle() const;
    [[nodiscard]] int32_t num_services() const;
    [[nodiscard]] std::vector<BleClientService> services() const;
    void set_connection(uint16_t id);
    [[nodiscard]] std::optional<Error> drop(ErrorCode reason);
    void reset();
    void add_service(const BleClientService& service);
private:
    uint16_t m_id;
    std::vector<BleClientService> m_services;

};


class CGap
{
public:
    CGap();
    ~CGap();
    CGap(const CGap& other) = delete;
    CGap(CGap&& other) noexcept;
    CGap& operator=(const CGap& other) = delete;
    CGap& operator=(CGap&& other);
public:
    void set_connection(uint16_t id);
    void reset_connection();
    void rssi();
    [[nodiscard]] uint16_t connection_handle() const ;
    [[nodiscard]] std::optional<Error> discover_services();
    [[nodiscard]] std::optional<Error> start();
    [[nodiscard]] std::optional<Error> begin_advertise();
    [[nodiscard]] std::optional<Error> end_advertise();
    [[nodiscard]] std::optional<Error> drop_connection(ErrorCode reason);
private:
    uint8_t m_bleAddressType;
    ble_gap_adv_params m_params;
    CConnectionHandle m_currentConnectionHandle;
};

} // namespace nimble
