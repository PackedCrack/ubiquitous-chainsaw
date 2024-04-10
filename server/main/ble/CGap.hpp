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
	enum class Event : uint8_t
	{
		connect = BLE_GAP_EVENT_CONNECT,
		disconnect = BLE_GAP_EVENT_DISCONNECT,
		update = BLE_GAP_EVENT_CONN_UPDATE,
		updateReq = BLE_GAP_EVENT_CONN_UPDATE_REQ,
		l2capUpdateReq = BLE_GAP_EVENT_L2CAP_UPDATE_REQ,
		termFailure = BLE_GAP_EVENT_TERM_FAILURE,
		disc = BLE_GAP_EVENT_DISC,
		discComplete = BLE_GAP_EVENT_DISC_COMPLETE,
		advertismentComplete = BLE_GAP_EVENT_ADV_COMPLETE,
		encryptionChanged = BLE_GAP_EVENT_ENC_CHANGE,
		passkeyAction = BLE_GAP_EVENT_PASSKEY_ACTION,
		notifyRecieve = BLE_GAP_EVENT_NOTIFY_RX,
		notifyTransfer = BLE_GAP_EVENT_NOTIFY_TX,
		subscribe = BLE_GAP_EVENT_SUBSCRIBE,
		mtu = BLE_GAP_EVENT_MTU,
		identityResolved = BLE_GAP_EVENT_IDENTITY_RESOLVED,
		repeatPairing = BLE_GAP_EVENT_REPEAT_PAIRING,
		physicalUpdateComplete = BLE_GAP_EVENT_PHY_UPDATE_COMPLETE,
		extDisc = BLE_GAP_EVENT_EXT_DISC, // external disconnect?
		periodicSync = BLE_GAP_EVENT_PERIODIC_SYNC,
		periodicSyncReport = BLE_GAP_EVENT_PERIODIC_REPORT,
		periodicSyncLost = BLE_GAP_EVENT_PERIODIC_SYNC_LOST,
		scanRequireRCVD = BLE_GAP_EVENT_SCAN_REQ_RCVD,
		periodicTransfer = BLE_GAP_EVENT_PERIODIC_TRANSFER,
		pathlossThreshold = BLE_GAP_EVENT_PATHLOSS_THRESHOLD,
		transmitPower = BLE_GAP_EVENT_TRANSMIT_POWER,
		subrateChange = BLE_GAP_EVENT_SUBRATE_CHANGE
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
	static int event_callback_caller(ble_gap_event* pEvent, function eventCallback);
    [[nodiscard]] std::optional<int8_t>  rssi() const;
	void set_connection(CConnection&& newConncetion);
	[[nodiscard]] std::optional<CConnection*> active_connection();
    [[nodiscard]] std::optional<CConnection::Error> drop_connection(CConnection::DropCode reason);
    [[nodiscard]] std::optional<Error> begin_advertise();
    [[nodiscard]] std::optional<Error> end_advertise();
	[[nodiscard]] bool is_advertising() const;
	[[nodiscard]] std::function<void(ble_gap_event*)> make_event_callback();
private:
    AddressType m_BleAddressType;
    ble_gap_adv_params m_Params;
    CConnection m_ActiveConnection;
	std::function<void(ble_gap_event*)> m_EventCallback;
};
} // namespace nimble
