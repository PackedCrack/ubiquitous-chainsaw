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
#include "ble_common.hpp"

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
struct Error
{
	ErrorCode code;
	std::string msg;
};
class CConnectionHandle // NOTE: Will this be needed for GATT services???? probably drop connectino if not authenticated
{
public:
	struct Error
	{
		ErrorCode code;
		std::string msg;
	};
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
	enum class Events : int32_t
	{
		connect = BLE_GAP_EVENT_CONNECT,
		disconnect = BLE_GAP_EVENT_DISCONNECT,
		update = BLE_GAP_EVENT_CONN_UPDATE,
		updateReq = BLE_GAP_EVENT_CONN_UPDATE_REQ

		//#define BLE_GAP_EVENT_L2CAP_UPDATE_REQ      5
		//#define BLE_GAP_EVENT_TERM_FAILURE          6
		//#define BLE_GAP_EVENT_DISC                  7
		//#define BLE_GAP_EVENT_DISC_COMPLETE         8
		//#define BLE_GAP_EVENT_ADV_COMPLETE          9
		//#define BLE_GAP_EVENT_ENC_CHANGE            10
		//#define BLE_GAP_EVENT_PASSKEY_ACTION        11
		//#define BLE_GAP_EVENT_NOTIFY_RX             12
		//#define BLE_GAP_EVENT_NOTIFY_TX             13
		//#define BLE_GAP_EVENT_SUBSCRIBE             14
		//#define BLE_GAP_EVENT_MTU                   15
		//#define BLE_GAP_EVENT_IDENTITY_RESOLVED     16
		//#define BLE_GAP_EVENT_REPEAT_PAIRING        17
		//#define BLE_GAP_EVENT_PHY_UPDATE_COMPLETE   18
		//#define BLE_GAP_EVENT_EXT_DISC              19
		//#define BLE_GAP_EVENT_PERIODIC_SYNC         20
		//#define BLE_GAP_EVENT_PERIODIC_REPORT       21
		//#define BLE_GAP_EVENT_PERIODIC_SYNC_LOST    22
		//#define BLE_GAP_EVENT_SCAN_REQ_RCVD         23
		//#define BLE_GAP_EVENT_PERIODIC_TRANSFER     24
		//#define BLE_GAP_EVENT_PATHLOSS_THRESHOLD    25
		//#define BLE_GAP_EVENT_TRANSMIT_POWER        26
		//#define BLE_GAP_EVENT_SUBRATE_CHANGE        27
	};
	struct Error
	{
		ErrorCode code;
		std::string msg;
	};
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
    [[nodiscard]] std::optional<CConnectionHandle::Error> discover_services();
    [[nodiscard]] std::optional<Error> start();
    [[nodiscard]] std::optional<Error> begin_advertise();
    [[nodiscard]] std::optional<Error> end_advertise();
    [[nodiscard]] std::optional<CConnectionHandle::Error> drop_connection(ErrorCode reason);
private:
    uint8_t m_BleAddressType;
    ble_gap_adv_params m_Params;
    CConnectionHandle m_CurrentConnectionHandle;
};
} // namespace nimble
