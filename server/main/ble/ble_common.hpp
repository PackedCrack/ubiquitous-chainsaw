#pragma once
#include "common/ble_services.hpp"
#include "../server_common.hpp"
// std
#include <cstring>
#include <string_view>
#include <array>
// nimble
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#pragma GCC diagnostic ignored "-Wconversion"
#include "host/ble_uuid.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#pragma GCC diagnostic pop
namespace ble
{
constexpr uint8_t INVALID_ADDRESS_TYPE = 255u;
constexpr int SUCCESS = 0;

enum class AddressType : uint8_t
{
    publicMac = BLE_ADDR_PUBLIC,
    randomMac = BLE_ADDR_RANDOM,
    invalid = INVALID_ADDRESS_TYPE
};
[[nodiscard]] constexpr std::string_view address_type_to_str(AddressType type)
{
    UNHANDLED_CASE_PROTECTION_ON
    switch (type)
    {
    case AddressType::publicMac:
    {
        return "publicMac";
    }
    case AddressType::randomMac:
    {
        return "randomMac";
    }
    case AddressType::invalid:
    {
        return "invalid";
    }
    }
    UNHANDLED_CASE_PROTECTION_OFF

    __builtin_unreachable();
}
enum class NimbleErrorCode : int32_t
{
    success = SUCCESS,                               // (not defined by nimble)
    temporaryFailure = BLE_HS_EAGAIN,                // Temporary failure; try again
    inProgressOrCompleted = BLE_HS_EALREADY,         // Operation already in progress or completed
    invalidArguments = BLE_HS_EINVAL,                // One or more arguments are invalid
    toSmallBuffer = BLE_HS_EMSGSIZE,                 // The provided buffer is too small
    noEntry = BLE_HS_ENOENT,                         // No entry matching the specified criteria
    resourceExhaustion = BLE_HS_ENOMEM,              // Operation failed due to resource exhaustion
    noConnection = BLE_HS_ENOTCONN,                  // No open connection with the specified handle
    operationDisabled = BLE_HS_ENOTSUP,              // Operation disabled at compile time
    unexpectedCallbackBehavior = BLE_HS_EAPP,        // Application callback behaved unexpectedly
    invalidPeerCommand = BLE_HS_EBADDATA,            // Command from peer is invalid
    osError = BLE_HS_EOS,                            // Mynewt OS error
    invalidControllerEvent = BLE_HS_ECONTROLLER,     // Event from controller is invalid
    operationTimeOut = BLE_HS_ETIMEOUT,              // Operation timed out
    operationCompleted = BLE_HS_EDONE,               // Operation completed successfully
    isBusy = BLE_HS_EBUSY,                           // Operation cannot be performed until procedure completes
    peerRejectedConnectionParam = BLE_HS_EREJECT,    // Peer rejected a connection parameter update request
    unexpectedFailure = BLE_HS_EUNKNOWN,             // Unexpected failure; catch all
    wrongRole = BLE_HS_EROLE,                        // Operation requires different role (e.g., central vs. peripheral)
    requestTimeOut = BLE_HS_ETIMEOUT_HCI,            // HCI request timed out; controller unresponsiv
    eventMemoryExhaustion =
        BLE_HS_ENOMEM_EVT,    // Controller failed to send event due to memory exhaustion (combined host-controller only)
    noConfiguredIdentityAddress = BLE_HS_ENOADDR,    // Operation requires an identity address but none configured
    notSynced = BLE_HS_ENOTSYNCED,                   // Attempt to use the host before it is synced with controller
    insufficientAuthen = BLE_HS_EAUTHEN,             // Insufficient authentication
    insufficientAuthor = BLE_HS_EAUTHOR,             // Insufficient authorization
    insufficientEncLvl = BLE_HS_EENCRYPT,            // Insufficient encryption level
    insufficientKeySize = BLE_HS_EENCRYPT_KEY_SZ,    // Insufficient key size
    storageFull = BLE_HS_ESTORE_CAP,                 // Storage at capacity
    storageIO = BLE_HS_ESTORE_FAIL,                  // Storage IO error
    preemptedOperation = BLE_HS_EPREEMPTED,          // Operation preempted
    disabledFeature = BLE_HS_EDISABLED,              // FDisabled feature
    operationStalled = BLE_HS_ESTALLED               // Operation stalled
};
enum class EspErrorCode : int32_t
{
    success = ESP_OK,                              /*!< esp_err_t value indicating success (no error) */
    fail = ESP_FAIL,                               /*!< Generic esp_err_t code indicating failure */
    noMemory = ESP_ERR_NO_MEM,                     /*!< Out of memory */
    invalidArg = ESP_ERR_INVALID_ARG,              /*!< Invalid argument */
    invalidState = ESP_ERR_INVALID_STATE,          /*!< Invalid state */
    invalidSize = ESP_ERR_INVALID_SIZE,            /*!< Invalid size */
    resourceNotFound = ESP_ERR_NOT_FOUND,          /*!< Requested resource not found */
    operationNotSupported = ESP_ERR_NOT_SUPPORTED, /*!< Operation or feature not supported */
    operationTimeOut = ESP_ERR_TIMEOUT,            /*!< Operation timed out */
    invalidResponse = ESP_ERR_INVALID_RESPONSE,    /*!< Received response was invalid */
    invalidCheckSum = ESP_ERR_INVALID_CRC,         /*!< CRC or checksum was invalid */
    invalidVersion = ESP_ERR_INVALID_VERSION,      /*!< Version was invalid */
    invalidMAC = ESP_ERR_INVALID_MAC,              /*!< MAC address was invalid */
    notFinished = ESP_ERR_NOT_FINISHED             /*!< There are items remained to retrieve */
};
[[nodiscard]] constexpr std::string_view nimble_error_to_string(NimbleErrorCode error)
{
    UNHANDLED_CASE_PROTECTION_ON
    switch (error)
    {
    case NimbleErrorCode::success:
    {
        return "Success";
    }
    case NimbleErrorCode::temporaryFailure:
    {
        return "Temporary failure; try again";
    }
    case NimbleErrorCode::inProgressOrCompleted:
    {
        return "Operation already in progress or completed";
    }
    case NimbleErrorCode::invalidArguments:
    {
        return "One or more arguments are invalid";
    }
    case NimbleErrorCode::toSmallBuffer:
    {
        return "The provided buffer is too small";
    }
    case NimbleErrorCode::noEntry:
    {
        return "No entry matching the specified criteria";
    }
    case NimbleErrorCode::resourceExhaustion:
    {
        return "Operation failed due to resource exhaustion";
    }
    case NimbleErrorCode::noConnection:
    {
        return "No open connection with the specified handle";
    }
    case NimbleErrorCode::operationDisabled:
    {
        return "Operation disabled at compile time";
    }
    case NimbleErrorCode::unexpectedCallbackBehavior:
    {
        return "Application callback behaved unexpectedly";
    }
    case NimbleErrorCode::invalidPeerCommand:
    {
        return "Command from peer is invalid";
    }
    case NimbleErrorCode::osError:
    {
        return "Mynewt OS error";
    }
    case NimbleErrorCode::invalidControllerEvent:
    {
        return "Event from controller is invalid";
    }
    case NimbleErrorCode::operationTimeOut:
    {
        return "Operation timed out";
    }
    case NimbleErrorCode::operationCompleted:
    {
        return "Operation completed successfully";
    }
    case NimbleErrorCode::isBusy:
    {
        return "Operation cannot be performed until procedure completes";
    }
    case NimbleErrorCode::peerRejectedConnectionParam:
    {
        return "Peer rejected a connection parameter update request";
    }
    case NimbleErrorCode::unexpectedFailure:
    {
        return "Unexpected failure; catch all";
    }
    case NimbleErrorCode::wrongRole:
    {
        return "Operation requires different role (e.g., central vs. peripheral)";
    }
    case NimbleErrorCode::requestTimeOut:
    {
        return "HCI request timed out; controller unresponsiv";
    }
    case NimbleErrorCode::eventMemoryExhaustion:
    {
        return "Controller failed to send event due to memory exhaustion (combined host-controller only)";
    }
    case NimbleErrorCode::noConfiguredIdentityAddress:
    {
        return "Operation requires an identity address but none configured";
    }
    case NimbleErrorCode::notSynced:
    {
        return "Attempt to use the host before it is synced with controller";
    }
    case NimbleErrorCode::insufficientAuthen:
    {
        return "Insufficient authentication";
    }
    case NimbleErrorCode::insufficientAuthor:
    {
        return "Insufficient authorization";
    }
    case NimbleErrorCode::insufficientEncLvl:
    {
        return "Insufficient encryption level";
    }
    case NimbleErrorCode::insufficientKeySize:
    {
        return "Insufficient key size";
    }
    case NimbleErrorCode::storageFull:
    {
        return "Storage at capacity";
    }
    case NimbleErrorCode::storageIO:
    {
        return "Storage IO error";
    }
    case NimbleErrorCode::preemptedOperation:
    {
        return "Operation preempted";
    }
    case NimbleErrorCode::disabledFeature:
    {
        return "FDisabled feature";
    }
    case NimbleErrorCode::operationStalled:
    {
        return "Operation stalled";
    }
    }
    UNHANDLED_CASE_PROTECTION_OFF

    __builtin_unreachable();
}
[[nodiscard]] inline ble_uuid128_t make_ble_uuid128(uint16_t uniqueValue)
{
    ble_uuid128_t uuid{};
    uuid.u.type = BLE_UUID_TYPE_128;

    ble::UUID base = BaseUUID;
    ble::UUID::apply_custom_id(base, uniqueValue);
    std::reverse(std::begin(base.data), std::end(base.data));

    static_assert(std::is_trivially_copy_constructible_v<decltype(uuid)>);
    static_assert(std::is_trivially_copyable_v<decltype(BaseUUID)>);
    static_assert(ARRAY_SIZE(uuid.value) == sizeof(decltype(BaseUUID)));
    // cppcheck-suppress sizeofDivisionMemfunc
    std::memcpy(&(uuid.value[0]), base.data.data(), ARRAY_SIZE(uuid.value));

    return uuid;
}
template<typename buffer_t>
requires common::const_buffer<buffer_t>
[[nodiscard]] NimbleErrorCode append_read_data(os_mbuf* om, buffer_t&& data)
{
    ASSERT(data.size() <= UINT16_MAX, "Buffer is too big!");
    ASSERT(om != nullptr, "A os buffer is required!");

    return NimbleErrorCode{ os_mbuf_append(om, data.data(), static_cast<uint16_t>(data.size())) };
}
[[nodiscard]] std::expected<std::string, ble::NimbleErrorCode> current_mac_address(AddressType type);
[[nodiscard]] std::expected<uint16_t, ble::NimbleErrorCode> chr_attri_handle(uint16_t svcUUID, uint16_t chrUUID);
}    // namespace ble
