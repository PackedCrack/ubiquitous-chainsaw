#pragma once

/* STD */
#include <string>
#include <cstdio>
#include <type_traits>
#include <vector>
#include <atomic>
#include <array>
#include <optional>


#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <utility>

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
    [[nodiscard]] int num_services() const;
    //[[nodiscard]] int num_characteristics(uint16_t handleStart, uint16_t handleEnd) const;
    [[nodiscard]] std::vector<BleClientService> services() const; // cannot be const because of the discovery process (we add characteristics afterwards)
    void set_connection(uint16_t id);
    [[nodiscard]] int drop(int reason);
    void reset();
    void add_service(const BleClientService& service);
private:
    uint16_t m_id;
    std::vector<BleClientService> m_services;

};



enum class ErrorCode : int32_t
{
	//success = Success,
	inProgress = BLE_HS_EALREADY,
	unknown = INT32_MAX
};
struct Error
{
	ErrorCode code;
	std::string msg;
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
    [[nodiscard]] uint16_t connection_handle() const ;
    [[nodiscard]] int drop_connection(int reason);
    [[nodiscard]] int discover_services();
    void set_connection(uint16_t id);
    void reset_connection();
    [[nodiscard]] int start();
    void rssi();
    [[nodiscard]] int begin_advertise();
    [[nodiscard]] std::optional<Error> end_advertise();
private:
    uint8_t m_bleAddressType;
    ble_gap_adv_params m_params;
    //std::atomic<bool> m_isAdvertising;
    CConnectionHandle m_currentConnectionHandle;
};

} // namespace nimble
