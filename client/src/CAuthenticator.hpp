#pragma once
#include "system/System.hpp"
#include "common/Pointer.hpp"
#include "common/CThreadSafeQueue.hpp"
#include "security/ecc_key.hpp"
#include "bluetoothLE/Device.hpp"


class CAuthenticator
{
private:
    struct Server
    {
        ble::CDevice device;
        ble::DeviceInfo info;
    };
public:
    explicit CAuthenticator(security::CEccPublicKey* pServerKey);
    ~CAuthenticator() = default;
    CAuthenticator(const CAuthenticator& other);
    CAuthenticator(CAuthenticator&& other) = default;
    CAuthenticator& operator=(const CAuthenticator& other);
    CAuthenticator& operator=(CAuthenticator&& other) = default;
    void copy(const CAuthenticator& other);
public:
    void enqueue_devices(const std::vector<ble::DeviceInfo>& infos);
    void deauth();
    [[nodiscard]] bool server_identified() const;
    [[nodiscard]] ble::CDevice server() const;
    [[nodiscard]] uint64_t server_address() const;
    [[nodiscard]] std::string server_address_as_str() const;
private:
    sys::fire_and_forget_t process_queue();
[[nodiscard]] sys::awaitable_t<bool> verify_server_address(const ble::CDevice& device, uint64_t address) const;
private:
    Pointer<security::CEccPublicKey> m_pServerKey = nullptr;
    CThreadSafeQueue<ble::DeviceInfo> m_Devices;
    std::optional<Server> m_AuthenticatedServer;
    std::unique_ptr<std::shared_mutex> m_pSharedMutex;
};