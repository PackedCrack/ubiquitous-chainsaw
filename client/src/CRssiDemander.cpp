//
// Created by qwerty on 2024-04-27.
//
#include "CRssiDemander.hpp"
#include "common/CCoroutineManager.hpp"
// clang-format off


// clang-format on
CRssiDemander::CRssiDemander(CServer& server, gfx::CWindow& window, std::chrono::seconds demandInterval)
    : m_Queue{}
    , m_DemandInterval{ demandInterval }
    , m_pServerPubKey{ load_key<security::CEccPublicKey>(SERVER_PUBLIC_KEY_NAME) }
    , m_pServer{ &server }
    , m_pWindow{ &window }
//, m_pMutex{ std::make_unique<std::mutex>() }
//, m_pShouldExit{ std::make_unique<std::atomic<bool>>(false) }
//, m_pCV{ std::make_unique<std::condition_variable>() }
//, m_RssiDemander{ std::thread{ make_rssi_demander() } }
{}
CRssiDemander::~CRssiDemander()
{
    //if (m_pServer)
    //{
    //    LOG_INFO("~CRssiDemander calling unsubscribe");
    //    m_pServer->unsubscribe();
    //}
    LOG_INFO("EXITING ~CRssiDemander");
    //if (m_pServer && m_pMutex)
    //{
    //    //    join_rssi_demander();
    //}
}
CRssiDemander::CRssiDemander(CRssiDemander&& other) noexcept
    : m_Queue{}
    , m_DemandInterval{}
    , m_pServerPubKey{}
    , m_pServer{}
    , m_pWindow{}
//, m_pMutex{}
//, m_pShouldExit{}
//, m_pCV{}
//, m_RssiDemander{}
{
    move(other);
}
CRssiDemander& CRssiDemander::operator=(CRssiDemander&& other) noexcept
{
    move(other);

    return *this;
}
void CRssiDemander::move(CRssiDemander& other)
{
    //other.join_rssi_demander();

    m_Queue = std::move(other.m_Queue);
    m_pServer = std::move(other.m_pServer);
    m_pServerPubKey = std::move(other.m_pServerPubKey);
    m_pWindow = std::move(other.m_pWindow);
    //m_pMutex = std::move(other.m_pMutex);
    m_DemandInterval = std::move(other.m_DemandInterval);
    //m_pShouldExit = std::move(other.m_pShouldExit);
    //m_RssiDemander = std::thread{ make_rssi_demander() };
}
std::optional<std::vector<int8_t>> CRssiDemander::rssi()
{
    auto vec = std::make_optional<std::vector<int8_t>>();
    while (!m_Queue.empty())
    {
        vec->push_back(0);
        m_Queue.pop(vec->back());
    }

    return vec->size() > 0 ? vec : std::nullopt;
}
auto CRssiDemander::make_rssi_receiver() const
{
    return [this](std::span<const uint8_t>) { LOG_INFO("DE E NAJS"); };
}
void CRssiDemander::send_demand()
{
    //if (m_Timer.lap<float>() >= static_cast<float>(m_DemandInterval.count()))
    //{
    //    if (!co_await m_pServer->has_subscribed())
    //    {
    //        m_pServer->subscribe(make_rssi_receiver());
    //    }
    //
    //    m_pServer->demand_rssi(*m_pWindow);
    //
    //    m_Timer.reset();
    //}

    auto& coroutineManager = common::coroutine_manager_instance();
    coroutineManager.fire_and_forget(
        [](std::weak_ptr<CRssiDemander> wpSelf) -> sys::awaitable_t<void>
        {
            std::shared_ptr<CRssiDemander> pSelf = wpSelf.lock();

            if (pSelf)
            {
                if (pSelf->m_Timer.lap<float>() >= static_cast<float>(pSelf->m_DemandInterval.count()))
                {
                    if (!co_await pSelf->m_pServer->has_subscribed())
                    {
                        LOG_INFO("NOT SUBSCRIBED");
                        pSelf->m_pServer->subscribe(pSelf->make_rssi_receiver());
                    }
                    else
                    {
                        LOG_INFO("SUBSCRIBED");
                    }

                    pSelf->m_pServer->demand_rssi(*(pSelf->m_pWindow));

                    pSelf->m_Timer.reset();
                }
            }
        },
        weak_from_this());
}
//std::function<void()> CRssiDemander::make_rssi_demander() const
//{
//    return [this]()
//    {
//        do
//        {
//            {
//                std::unique_lock lock{ *(m_pMutex) };
//                if (m_pServer->is_authenticated())
//                {
//                    static int32_t a = 0;
//                    if (a < 1)
//                    // if not subscribed
//                    {
//                        m_pServer->subscribe(make_rssi_receiver());
//                        ++a;
//                    }
//                    m_pServer->demand_rssi(*m_pWindow);
//                }
//            }
//
//            std::unique_lock lock{ *(m_pMutex) };
//            m_pCV->wait_for(lock, m_DemandInterval);
//        } while (!m_pShouldExit->load());
//    };
//}
//void CRssiDemander::join_rssi_demander() noexcept
//{
//    {
//        std::lock_guard lock{ *(m_pMutex) };
//        m_pServer->unsubscribe();
//
//        bool expected = false;
//        while (!m_pShouldExit->compare_exchange_strong(expected, true))
//        {};
//        m_pCV->notify_one();
//    }
//
//    m_RssiDemander.join();
//}
