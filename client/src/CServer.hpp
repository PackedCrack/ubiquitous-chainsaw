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
class CServer
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
    void demand_rssi(gfx::CWindow& window, /*CAuthenticator& authenticator,*/ security::CEccPrivateKey* pKey) const;
    [[nodiscard]] bool connected() const;
    [[nodiscard]] bool is_authenticated() const;
    [[nodiscard]] uint64_t server_address() const;
    [[nodiscard]] std::string server_address_as_str() const;
private:
    std::unique_ptr<mutex_type> m_pMutex;
    std::optional<AuthenticatedDevice> m_Server;
};
