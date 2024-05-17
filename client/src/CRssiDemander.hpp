//
// Created by qwerty on 2024-04-27.
//
#pragma once
#include "common/client_common.hpp"
#include "common/Pointer.hpp"
#include "common/CStopWatch.hpp"
#include "common/CThreadSafeQueue.hpp"
#include "CServer.hpp"
#include "CReplayProtector.hpp"
//
//
//
//
class CRssiDemander : public std::enable_shared_from_this<CRssiDemander>
{
public:
    CRssiDemander(std::chrono::seconds demandInterval, CServer& server, gfx::CWindow& window);
    ~CRssiDemander() = default;
    CRssiDemander(const CRssiDemander& other) = delete;
    CRssiDemander(CRssiDemander&& other) = default;
    CRssiDemander& operator=(const CRssiDemander& other) = delete;
    CRssiDemander& operator=(CRssiDemander&& other) = default;
public:
    [[nodiscard]] std::optional<std::vector<int8_t>> rssi();
    void send_demand();
private:
    void demand_rssi();
    [[nodiscard]] auto make_rssi_receiver();
    [[nodiscard]] sys::awaitable_t<void> try_demand_rssi(std::weak_ptr<ble::CCharacteristic> characteristic, std::vector<byte> packet);
    [[nodiscard]] std::vector<byte> make_packet_demand_rssi();
private:
    CThreadSafeQueue<int8_t> m_Queue;
    std::chrono::seconds m_DemandInterval;
    std::unique_ptr<security::CEccPublicKey> m_pServerPubKey;
    std::unique_ptr<security::CEccPrivateKey> m_pClientPrivKey;
    Pointer<CServer> m_pServer = nullptr;
    Pointer<gfx::CWindow> m_pWindow = nullptr;
    common::CStopWatch<std::chrono::seconds> m_Timer;
    CReplayProtector m_Protector;
};
template<typename... ctor_args_t>
[[nodiscard]] std::shared_ptr<CRssiDemander> make_rssi_demander(ctor_args_t&&... args)
{
    return std::make_shared<CRssiDemander>(std::chrono::seconds(1), std::forward<ctor_args_t>(args)...);
}
