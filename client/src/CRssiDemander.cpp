//
// Created by qwerty on 2024-04-27.
//
#include "CRssiDemander.hpp"
#include "common/CCoroutineManager.hpp"
//
//
//
//
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
void insert_data(std::span<uint8_t> dst, std::span<const uint8_t> src)
{
    ASSERT(dst.size() == src.size(), "Buffer size mismatch when calling memcpy");
    size_t bytesToCopy = src.size() < dst.size() ? src.size() : dst.size();
    std::memcpy(dst.data(), src.data(), bytesToCopy);
}
[[nodiscard]] std::vector<byte> insert_random_data_block(std::vector<byte>& packet, const std::vector<byte>& randomBlock)
{
    static constexpr ble::DemandRSSIHeader HEADER = ble::header_whereami_demand_rssi();

    packet[HEADER.randomDataOffset] = sizeof(decltype(HEADER));
    packet[HEADER.randomDataSize] = common::assert_down_cast<uint8_t>(randomBlock.size());

    std::span<uint8_t> packetRandomBlock{ std::begin(packet) + packet[HEADER.randomDataOffset], packet[HEADER.randomDataSize] };
    insert_data(packetRandomBlock, randomBlock);

    return packet;
}
template<typename sha_t>
requires security::hash_algorithm<sha_t>
[[nodiscard]] std::vector<byte> insert_hash(std::vector<byte>& packet, const security::CHash<sha_t>& hash)
{
    static constexpr ble::DemandRSSIHeader HEADER = ble::header_whereami_demand_rssi();

    packet[HEADER.hashOffset] = sizeof(decltype(HEADER)) + packet[HEADER.randomDataSize];
    packet[HEADER.hashSize] = common::assert_down_cast<uint8_t>(hash.size());

    std::span<uint8_t> packetHashBlock{ std::begin(packet) + packet[HEADER.hashOffset], packet[HEADER.hashSize] };
    insert_data(packetHashBlock, std::span<const uint8_t>{ hash.data(), hash.size() });

    return packet;
}
[[nodiscard]] std::vector<byte> insert_signature(std::vector<byte>& packet, const std::vector<byte>& signature)
{
    static constexpr ble::DemandRSSIHeader HEADER = ble::header_whereami_demand_rssi();

    packet[HEADER.signatureOffset] = sizeof(decltype(HEADER)) + packet[HEADER.randomDataSize] + packet[HEADER.hashSize];
    packet[HEADER.signatureSize] = common::assert_down_cast<uint8_t>(signature.size());

    std::span<uint8_t> packetSignatureBlock{ std::begin(packet) + packet[HEADER.signatureOffset], packet[HEADER.signatureSize] };
    insert_data(packetSignatureBlock, signature);

    return packet;
}
}    // namespace
CRssiDemander::CRssiDemander(std::chrono::seconds demandInterval, CServer& server, gfx::CWindow& window)
    : m_Queue{}
    , m_DemandInterval{ demandInterval }
    , m_pServerPubKey{ load_key<security::CEccPublicKey>(SERVER_PUBLIC_KEY_NAME) }
    , m_pClientPrivKey{ load_key<security::CEccPrivateKey>(CLIENT_PRIVATE_KEY_NAME) }
    , m_pServer{ &server }
    , m_pWindow{ &window }
    , m_Timer{}
    , m_Protector{ std::chrono::seconds(3) }
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
            ble::ShaVersion version = extract_sha_version(packet);
            if (valid_sha_version_id(version))
            {
                if (pSelf->m_Protector.expected_random_data(view_random_data_block(packet)))
                {
                    ble::ShaHash hash = ble::make_sha_hash(version, view_hash(packet));
                    std::span<uint8_t> signatureBlock = view_signature(packet);

                    if (valid_signature(pSelf->m_pServerPubKey.get(), signatureBlock, hash))
                    {
                        int8_t rssi = extract_rssi_value(packet);
                        pSelf->m_Queue.push(rssi);
                    }
                    else
                    {
                        // TODO:: missed answers counter
                        LOG_WARN("Recieved RSSI Notification packet with an invalid signature!");
                    }
                }
                else
                {
                    LOG_WARN("Recieved packet with unexpected random data..");
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
                // demand_rssi spawns a new coroutine task because we can't wait here.
                // If we wait here then the timer will not be reset at the correct time.
                pSelf->demand_rssi();
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
                [[fallthrough]];
            }
            case CServer::HasSubscribedResult::inFlight:
            {
                break;
            }
            }
            UNHANDLED_CASE_PROTECTION_OFF
        }
    };

    coroutineManager.fire_and_forget(coroutine, weak_from_this());
}
void CRssiDemander::demand_rssi()
{
    auto& coroutineManager = common::coroutine_manager_instance();
    auto coroutine = [](std::weak_ptr<CRssiDemander> wpSelf) -> sys::awaitable_t<void>
    {
        std::shared_ptr<CRssiDemander> pSelf = wpSelf.lock();
        if (!pSelf)
        {
            co_return;
        }
        std::optional<std::shared_ptr<ble::CDevice>> device = pSelf->m_pServer->device();
        if (!device)
        {
            co_return;
        }
        std::shared_ptr<ble::CDevice> pDevice = std::move(device.value());
        if (!pDevice)
        {
            co_return;
        }

        std::optional<std::weak_ptr<ble::CCharacteristic>> wpCharacteristic =
            pDevice->characteristic(ble::uuid_service_whereami(), ble::uuid_characteristic_whereami_demand_rssi());
        if (!wpCharacteristic)
        {
            LOG_ERROR("Could not find Service: \"WhereAmI\" or Characteristic: \"Demand RSSI\" when trying to demand RSSI.");
            co_return;
        }

        if (pSelf->m_pClientPrivKey)
        {
            co_await pSelf->try_demand_rssi(wpCharacteristic.value(), pSelf->make_packet_demand_rssi());
        }
    };

    coroutineManager.fire_and_forget(coroutine, weak_from_this());
}
sys::awaitable_t<void> CRssiDemander::try_demand_rssi(std::weak_ptr<ble::CCharacteristic> wpCharacteristic, std::vector<byte> packet)
{
    const std::shared_ptr<ble::CCharacteristic>& pCharacteristic = wpCharacteristic.lock();

    static constexpr int32_t MAX_ATTEMPS = 1;
    int32_t attempt{};
    do
    {
        auto communicationStatus = co_await pCharacteristic->write_data(packet);

        UNHANDLED_CASE_PROTECTION_ON
        switch (communicationStatus)
        {
        case ble::CommunicationStatus::unreachable:
        {
            m_pServer->revoke_authentication();
            m_pWindow->popup_warning("Unreachable", "Could not demand RSSI value from server");
            [[fallthrough]];
        }
        case ble::CommunicationStatus::success:
        {
            attempt = MAX_ATTEMPS;
            break;
        }
        case ble::CommunicationStatus::accessDenied:
        {
            LOG_WARN("Could not write to WhereAmI's demand RSSI characteristic - Access Was Denied");
            m_pWindow->popup_warning("Access Denied", "Could not demand RSSI value from server");
            [[fallthrough]];
        }
        case ble::CommunicationStatus::protocolError:
        {
            ++attempt;
            LOG_WARN("Could not write to WhereAmI's demand RSSI characteristic - Protocol Error");
            m_pWindow->popup_warning("Protocol Error", "Could not demand RSSI value from server");
        }
        }
        UNHANDLED_CASE_PROTECTION_OFF
    } while (attempt < MAX_ATTEMPS);
}
std::vector<byte> CRssiDemander::make_packet_demand_rssi()
{
    using sha_type = security::Sha2_256;
    static std::expected<security::CRandom, security::CRandom::Error> expected = security::CRandom::make_rng();
    if (!expected)
    {
        LOG_FATAL("Failed to create cryptographic rng generator");
    }

    security::CRandom& rng = *expected;
    std::vector<byte> randomBlock = m_Protector.generate_random_block();
    security::CHash<sha_type> hash{ randomBlock };
    std::vector<byte> signature = m_pClientPrivKey->sign_hash(rng, hash);

    static constexpr ble::DemandRSSIHeader HEADER = ble::header_whereami_demand_rssi();
    const size_t PACKET_SIZE = sizeof(decltype(HEADER)) + randomBlock.size() + hash.size() + signature.size();
    ASSERT(PACKET_SIZE < 216, "Max packet size for BLE is around 216 bytes - give or take");

    std::vector<byte> packet{};
    packet.resize(PACKET_SIZE);
    packet[HEADER.shaVersion] = ble::sha_version_id<typename decltype(hash)::hash_type>();
    packet = insert_random_data_block(packet, randomBlock);
    packet = insert_hash(packet, hash);
    packet = insert_signature(packet, signature);

    return packet;
}
