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
    std::shared_ptr<ble::CDevice> pDevice;
    ble::DeviceInfo info;
};
class CServer
{
public:
    enum class HasSubscribedResult
    {
        subscribed,
        notSubscribed,
        notAuthenticated,
        inFlight
    };
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
    void subscribe(std::function<void(std::span<uint8_t>)>&& cb);
    void unsubscribe();
    void demand_rssi(gfx::CWindow& window);
    [[nodiscard]] bool connected() const;
    [[nodiscard]] bool is_authenticated() const;
    [[nodiscard]] sys::awaitable_t<HasSubscribedResult> has_subscribed() const;
    [[nodiscard]] uint64_t server_address() const;
    [[nodiscard]] std::string server_address_as_str() const;
private:
    [[nodiscard]] sys::awaitable_t<void>
        try_demand_rssi(gfx::CWindow* pWindow, std::weak_ptr<ble::CCharacteristic> characteristic, std::vector<byte> packet);
public:
    std::unique_ptr<mutex_type> m_pMutex;
    std::unique_ptr<security::CEccPrivateKey> m_pClientPrivateKey = nullptr;
    std::optional<AuthenticatedDevice> m_Server;
    std::function<void(std::span<const uint8_t>)> m_RssiReceiver;
};
