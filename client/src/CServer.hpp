//
// Created by qwerty on 2024-04-25.
//
#pragma once
#include "system/System.hpp"
#include "common/CThreadSafeQueue.hpp"
#include "bluetoothLE/Device.hpp"
#include "bluetoothLE/ble_common.hpp"
#include "gfx/CWindow.hpp"
// security
#include "security/ecc_key.hpp"
//
//
//
//
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
    void enqueue_devices(const std::vector<ble::DeviceInfo>& infos);
    void grant_authentication(AuthenticatedDevice&& device);
    void revoke_authentication();
    void subscribe(std::function<void(std::span<uint8_t>)>&& cb);
    void unsubscribe();
    [[nodiscard]] bool reload_public_key();
    [[nodiscard]] bool connected() const;
    [[nodiscard]] bool is_authenticated() const;
    [[nodiscard]] sys::awaitable_t<HasSubscribedResult> has_subscribed() const;
    [[nodiscard]] uint64_t server_address() const;
    [[nodiscard]] std::string server_address_as_str() const;
    [[nodiscard]] std::optional<std::shared_ptr<ble::CDevice>> device();
private:
    [[nodiscard]] auto make_connection_changed_cb();
    void process_queue();
    [[nodiscard]] sys::awaitable_t<bool> verify_server_address(const std::shared_ptr<ble::CDevice>&, uint64_t address) const;
public:
    std::unique_ptr<mutex_type> m_pMutex;
    std::optional<AuthenticatedDevice> m_Server;
    std::unique_ptr<security::CEccPublicKey> m_pServerKey = nullptr;
    CThreadSafeQueue<ble::DeviceInfo> m_Devices;
};
