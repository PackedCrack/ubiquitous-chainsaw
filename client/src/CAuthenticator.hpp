#pragma once
#include "system/System.hpp"
#include "common/Pointer.hpp"
#include "common/CThreadSafeQueue.hpp"
#include "security/ecc_key.hpp"
#include "bluetoothLE/Device.hpp"
#include "bluetoothLE/ble_common.hpp"
#include "CServer.hpp"
// clang-format off


// clang-format on
class CAuthenticator
{
public:
    explicit CAuthenticator(std::shared_ptr<CServer> pServer);
    ~CAuthenticator() = default;
    CAuthenticator(const CAuthenticator& other);
    CAuthenticator(CAuthenticator&& other) = default;
    CAuthenticator& operator=(const CAuthenticator& other);
    CAuthenticator& operator=(CAuthenticator&& other) = default;
public:
    void enqueue_devices(const std::vector<ble::DeviceInfo>& infos);
    void deauth();
    [[nodiscard]] bool server_identified() const;
    [[nodiscard]] uint64_t server_address() const;
    [[nodiscard]] std::string server_address_as_str() const;
private:
    //sys::fire_and_forget_t process_queue();
    [[nodiscard]] auto make_connection_changed_cb() const;
    void process_queue();
    [[nodiscard]] sys::awaitable_t<bool> verify_server_address(const ble::CDevice& device, uint64_t address) const;
private:
    std::unique_ptr<security::CEccPublicKey> m_pServerKey = nullptr;
    std::shared_ptr<CServer> m_pServer = nullptr;
    CThreadSafeQueue<ble::DeviceInfo> m_Devices;
};
