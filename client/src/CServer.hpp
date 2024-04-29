//
// Created by qwerty on 2024-04-25.
//
#pragma once
#include "system/System.hpp"
#include "bluetoothLE/Device.hpp"
#include "bluetoothLE/ble_common.hpp"
#include "gfx/CWindow.hpp"
// security
#include "security/ecc_key.hpp"
// clang-format off


// clang-format on
struct AuthenticatedDevice
{
    ble::CDevice device;
    ble::DeviceInfo info;
};
class CServer : public std::enable_shared_from_this<CServer>
{
private:
    using mutex_type = std::mutex;
public:
    CServer();
    ~CServer() = default;
    CServer(const CServer& other);
    CServer(CServer&& other) = default;
    CServer& operator=(const CServer& other);
    CServer& operator=(CServer&& other) = default;
public:
    void grant_authentication(AuthenticatedDevice&& device);
    void revoke_authentication();
    //sys::fire_and_forget_t subscribe(std::function<void(std::span<const uint8_t>)> cb);
    void subscribe(std::function<void(std::span<const uint8_t>)>&& cb);
    //sys::fire_and_forget_t unsubscribe();
    void unsubscribe();
    //sys::fire_and_forget_t demand_rssi(gfx::CWindow& window);
    void demand_rssi(gfx::CWindow& window);
    [[nodiscard]] bool connected() const;
    [[nodiscard]] bool is_authenticated() const;
    [[nodiscard]] sys::awaitable_t<bool> has_subscribed() const;
    [[nodiscard]] uint64_t server_address() const;
    [[nodiscard]] std::string server_address_as_str() const;
private:
    sys::awaitable_t<void>
        try_demand_rssi(gfx::CWindow* pWindow, std::weak_ptr<ble::CCharacteristic> characteristic, std::vector<byte> packet);
    [[nodiscard]] std::shared_ptr<ble::CService> get_service_whereami(std::string_view errorMsg) const;
    //[[nodiscard]] auto make_rssi_receiver();
    //sys::fire_and_forget_t subscribe_to_rssi_notification();
private:
    std::unique_ptr<mutex_type> m_pMutex;
    std::unique_ptr<security::CEccPrivateKey> m_pClientPrivateKey = nullptr;
    std::optional<AuthenticatedDevice> m_Server;
    std::function<void(std::span<const uint8_t>)> m_RssiReceiver;
};
