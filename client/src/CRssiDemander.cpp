//
// Created by qwerty on 2024-04-27.
//
#include "CRssiDemander.hpp"
#include "common/CCoroutineManager.hpp"
// clang-format off


// clang-format on
namespace
{
[[nodiscard]] bool valid_sha_version_id(ble::ShaVersion version)
{
    if (ble::sha_version_id(version) < ble::sha_version_id(ble::ShaVersion::count))
    {
        return true;
    }

    LOG_WARN_FMT("Recieved a packet with an invalid ShaVersion: {}", ble::sha_version_id(version));
    return false;
}
[[nodiscard]] bool valid_signature(security::CEccPublicKey* pServerKey, std::span<uint8_t> signature, ble::ShaHash& hash)
{
    bool verified{ false };
    std::visit([&verified, pServerKey, signature](auto&& h) { verified = pServerKey->verify_hash(signature, h); }, hash);

    return verified;
}
[[nodiscard]] std::span<uint8_t> view(std::span<uint8_t> packet, uint8_t offset, uint8_t size)
{
    ASSERT(packet.size() >= static_cast<std::size_t>(offset + size), "View is outside the packets buffer!");
    return std::span<uint8_t>{ std::begin(packet) + offset, size };
}
[[nodiscard]] std::span<uint8_t> view_random_data_block(std::span<uint8_t> packet)
{
    static constexpr ble::RSSINotificationHeader HEADER{};
    uint8_t offset = packet[HEADER.randomDataOffset];
    uint8_t size = packet[HEADER.randomDataSize];

    return view(packet, offset, size);
}
[[nodiscard]] std::span<uint8_t> view_signature(std::span<uint8_t> packet)
{
    static constexpr ble::RSSINotificationHeader HEADER{};
    uint8_t offset = packet[HEADER.signatureOffset];
    uint8_t size = packet[HEADER.signatureSize];

    return view(packet, offset, size);
}
[[nodiscard]] std::span<uint8_t> view_hash(std::span<uint8_t> packet)
{
    static constexpr ble::RSSINotificationHeader HEADER{};
    uint8_t offset = packet[HEADER.hashOffset];
    uint8_t size = packet[HEADER.hashSize];

    return view(packet, offset, size);
}
[[nodiscard]] ble::ShaVersion extract_sha_version(std::span<uint8_t> packet)
{
    static constexpr ble::RSSINotificationHeader HEADER{};
    return ble::ShaVersion{ packet[HEADER.shaVersion] };
}
[[nodiscard]] int8_t extract_rssi_value(std::span<uint8_t> packet)
{
    static constexpr ble::RSSINotificationHeader HEADER{};
    ASSERT(packet[HEADER.rssiSize] == 1, "Expected RSSI value to be sizeof 1 (int8_t).");
    uint8_t offset = packet[HEADER.rssiOffset];
    return static_cast<int8_t>(packet[offset]);
}
}    // namespace
CRssiDemander::CRssiDemander(CServer& server, gfx::CWindow& window, std::chrono::seconds demandInterval)
    : m_Queue{}
    , m_DemandInterval{ demandInterval }
    , m_pServerPubKey{ load_key<security::CEccPublicKey>(SERVER_PUBLIC_KEY_NAME) }
    , m_pServer{ &server }
    , m_pWindow{ &window }
    , m_Timer{}
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
    : m_Queue{ std::move(other.m_Queue) }
    , m_DemandInterval{ std::move(other.m_DemandInterval) }
    , m_pServerPubKey{ std::move(other.m_pServerPubKey) }
    , m_pServer{ std::move(other.m_pServer) }
    , m_pWindow{ std::move(other.m_pWindow) }
    , m_Timer{ std::move(other.m_Timer) }
{}
CRssiDemander& CRssiDemander::operator=(CRssiDemander&& other) noexcept
{
    m_Queue = std::move(other.m_Queue);
    m_pServer = std::move(other.m_pServer);
    m_pServerPubKey = std::move(other.m_pServerPubKey);
    m_pWindow = std::move(other.m_pWindow);
    m_DemandInterval = std::move(other.m_DemandInterval);
    m_Timer = std::move(other.m_Timer);

    return *this;
}
//void CRssiDemander::move(CRssiDemander& other)
//{
//    m_Queue = std::move(other.m_Queue);
//    m_pServer = std::move(other.m_pServer);
//    m_pServerPubKey = std::move(other.m_pServerPubKey);
//    m_pWindow = std::move(other.m_pWindow);
//    m_DemandInterval = std::move(other.m_DemandInterval);
//}
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
auto CRssiDemander::make_rssi_receiver()
{
    return [wpSelf = weak_from_this()](std::span<uint8_t> packet)
    {
        std::shared_ptr<CRssiDemander> pSelf = wpSelf.lock();
        if (pSelf)
        {
            static constexpr ble::RSSINotificationHeader HEADER{};
            uint8_t offset = packet[HEADER.hashOffset];
            uint8_t size = packet[HEADER.hashSize];
            std::span<uint8_t> hashBlock{ std::begin(packet) + offset, size };
            security::CHash<security::Sha2_256> hash{ std::cbegin(hashBlock), std::cend(hashBlock) };

            ble::ShaVersion version = extract_sha_version(packet);
            if (valid_sha_version_id(version))
            {
                std::span<uint8_t> randomData = view_random_data_block(packet);
                // TODO: Check cache if we expect an incoming packet with this random data

                ble::ShaHash hash = ble::make_sha_hash(version, view_hash(packet));
                std::span<uint8_t> signatureBlock = view_signature(packet);

                if (valid_signature(pSelf->m_pServerPubKey.get(), signatureBlock, hash))
                {
                    int8_t rssi = extract_rssi_value(packet);
                    LOG_INFO_FMT("Recieved verified RSSI Notification packet. RSSI Value: \"{}\"", rssi);
                    pSelf->m_Queue.push(rssi);
                }
                else
                {
                    LOG_WARN("Recieved RSSI Notification packet with an invalid signature!");
                }
            }
        }
    };
}
void CRssiDemander::send_demand()
{
    auto& coroutineManager = common::coroutine_manager_instance();
    auto coroutine = [](std::weak_ptr<CRssiDemander> wpSelf) -> sys::awaitable_t<void>
    {
        std::shared_ptr<CRssiDemander> pSelf = wpSelf.lock();
        if (!pSelf)
        {
            co_return;
        }

        float demandInterval = static_cast<float>(pSelf->m_DemandInterval.count());
        if (pSelf->m_Timer.lap<float>() >= demandInterval)
        {
            CServer::HasSubscribedResult result = co_await pSelf->m_pServer->has_subscribed();
            UNHANDLED_CASE_PROTECTION_ON
            switch (result)
            {
            case CServer::HasSubscribedResult::subscribed:
            {
                gfx::CWindow& window = *(pSelf->m_pWindow);
                pSelf->m_pServer->demand_rssi(window);
                pSelf->m_Timer.reset();
                break;
            }
            case CServer::HasSubscribedResult::notSubscribed:
            {
                pSelf->m_pServer->subscribe(pSelf->make_rssi_receiver());
                break;
            }
            case CServer::HasSubscribedResult::notAuthenticated:
            {
#ifndef NDEBUG
                //LOG_INFO("has_subscribed returned \"notAuthenticated\" - RSSI Demand will not be sent.");
#endif
                break;
            }
            case CServer::HasSubscribedResult::inFlight:
            {
#ifndef NDEBUG
                //LOG_INFO("has_subscribed returned \"inFlight\" - RSSI Demand will not be sent.");
#endif
                break;
            }
            }
            UNHANDLED_CASE_PROTECTION_OFF
        }
    };

    coroutineManager.fire_and_forget(coroutine, weak_from_this());
}
