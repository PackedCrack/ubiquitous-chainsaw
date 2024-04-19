#pragma once
#include "system/System.hpp"
#include "common/Pointer.hpp"
#include "security/ecc_key.hpp"
#include "bluetoothLE/ble_common.hpp"



class CAuthenticator
{
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
    [[nodiscard]] bool server_identified() const;
    [[nodiscard]] std::string server_address() const;
private:
    sys::fire_and_forget_t process_queue();
    [[nodiscard]] sys::awaitable_t<bool> verify_server_address(ble::DeviceInfo info) const;
private:
    Pointer<security::CEccPublicKey> m_pServerKey = nullptr;
    std::deque<ble::DeviceInfo> m_Devices;
    std::optional<ble::DeviceInfo> m_ServerInfo;
    std::unique_ptr<std::shared_mutex> m_pSharedMutex;
};