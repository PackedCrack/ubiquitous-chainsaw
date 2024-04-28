//
// Created by qwerty on 2024-04-27.
//
#pragma once
#include "client_common.hpp"
#include "common/Pointer.hpp"
#include "common/CStopWatch.hpp"
#include "common/CThreadSafeQueue.hpp"
#include "CServer.hpp"
// clang-format off


// clang-format on
class CRssiDemander
{
public:
    CRssiDemander(CServer& server, gfx::CWindow& window, std::chrono::seconds demandInterval);
    ~CRssiDemander();
    CRssiDemander(const CRssiDemander& other) = delete;
    CRssiDemander(CRssiDemander&& other) noexcept;
    CRssiDemander& operator=(const CRssiDemander& other) = delete;
    CRssiDemander& operator=(CRssiDemander&& other) noexcept;
private:
    void move(CRssiDemander& other);
public:
    [[nodiscard]] std::optional<std::vector<int8_t>> rssi();
    sys::fire_and_forget_t send_demand();
private:
    [[nodiscard]] auto make_rssi_receiver() const;
    //[[nodiscard]] std::function<void()> make_rssi_demander() const;
    //void join_rssi_demander() noexcept;
private:
    CThreadSafeQueue<int8_t> m_Queue;
    std::chrono::seconds m_DemandInterval;
    std::unique_ptr<security::CEccPublicKey> m_pServerPubKey;
    Pointer<CServer> m_pServer = nullptr;
    Pointer<gfx::CWindow> m_pWindow = nullptr;
    //std::unique_ptr<std::mutex> m_pMutex;
    //std::unique_ptr<std::atomic<bool>> m_pShouldExit;
    //std::unique_ptr<std::condition_variable> m_pCV;
    //std::thread m_RssiDemander;

    common::CStopWatch<std::chrono::seconds> m_Timer;
};
