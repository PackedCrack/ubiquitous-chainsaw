#pragma once
/* Project */
#include "ble_common.hpp"
#include "CConnection.hpp"
/* STD */
#include <type_traits>
#include <vector>
#include <array>
#include <optional>
#include <functional>
//#include <format> // apparently i dont have c++20

//#include <utility>
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
class CGap
{
	typedef void* function;
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
		NimbleErrorCode code;
		std::string msg;
	};
public:
    CGap();
    ~CGap();
    CGap(const CGap& other) = delete;
    CGap(CGap&& other) noexcept;
    CGap& operator=(const CGap& other) = delete;
    CGap& operator=(CGap&& other) noexcept;
public:
	static void event_callback_caller(ble_gap_event* pEvent, function eventCallback);
    void rssi();
	void set_connection(CConnection&& newConncetion);
	[[nodiscard]] std::optional<CConnection*> active_connection();
    [[nodiscard]] std::optional<CConnection::Error> drop_connection(CConnection::DropCode reason);
    [[nodiscard]] std::optional<Error> begin_advertise();
    [[nodiscard]] std::optional<Error> end_advertise();
	[[nodiscard]] bool is_advertising() const;
private:
    uint8_t m_BleAddressType;
    ble_gap_adv_params m_Params;
    CConnection m_ActiveConnection;
	std::function<void(ble_gap_event*)> m_EventCallback;
};
} // namespace nimble
