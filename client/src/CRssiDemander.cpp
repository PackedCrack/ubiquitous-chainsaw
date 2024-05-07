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
[[nodiscard]] std::vector<byte> generate_random_block(security::CRandom& rng)
{
    static std::random_device rd{};
    static std::mt19937_64 generator{ rd() };
    static std::uniform_int_distribution<size_t> distribution{ 64u, 96u };
    size_t blockSize = distribution(generator);

    std::expected<std::vector<byte>, security::CRandom::Error> expected = rng.generate_block(blockSize);
    ASSERT(expected, "Generating random data should never fail");

    // This is NOT a bug
    // https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QAJgAMfAQQCqAZ0wAFAB6cpvAFYgupWkwahkAUgkAhcxdLL6yAngGVG6AMKpaAVxYNJpVwAZPAZMADkfACNMYkkANlIAB1RFQicGD29ff2TUxwFg0IiWaNiJBLtMB3ShAiZiAkyfPwlbTHt8hlr6gkLwqJj42zqGpuzWxRHekP6SwfKASltUL2Jkdg5zAGYQ5G8sAGpTLbdkSfxBY%2BxTKQBBbd39zCOTyeIQ4AB9ADc8TAB3K43e4SHYMPZeQ7HNyYNSJKoETDoIF3B7gp4vNzfBEkFH3O7nEAgbEOXEnZAIepXA7ATAELgQBbA0wAdisdwOnIOxDpqwYRzZ%2BgOrQOW1IBwALKyACLHdn3Fmy1EEgjoIkkojEaEUqlbbA0ukSRnMtnArkHQkgWHwhxI6GWjVk06UrV68UhAjU60I1lWQXikViyUykNbeVmrk8gh8i2qoksVDYiDehwAOm%2BYi8mEZTLDJqV%2BNuDpxruduv1tIIW2NqNNHK5lpTiORrzjxJL2pdV3dlz1Bybvt9QsD4qlitD4frnKjMab6cz2dz4fHytuHoOLCYIQg6/qwGQ4p1xAAVMeDnvvkza/LzWgGJNrcRY2qQKkAF6YT4EA0EABqMVSAQXmlYU5WZKcDjwKgDjvB84WIaA6X/YhAP5Y5ZS2ECuCvW5zUHCDzWLUlSyPakSWAn8GSXCNzWnXliH5ElU3fRcwIgkMIPaZRIOg2DEXgxC/wA9JgPQ4UcLwutcNozkiM1TtywOEkJAoysjWogjI3oxiqgkZi8A/Y083YldpM5LjnigmCBDgxIEMrZDUNEzDRQkrl8LM2i5KdUi%2BxJLZVLpasNM880ZwYpSqi2fTDJCySCzw2tpQ4JZaE4ABWXg/A4LRSFQTg3GsawLRWNZnm2HhSAITQUqWABrEAJS4VMJHSiQJQlOIuAADi2ABOdKtikdL0oMTgJSymq8s4XhFBAGRqpylLSDgWAkDQFhEjoGJyEoDatvoWJvmQRJEh%2BLg%2Bs%2BLZus%2BNQ4glPg6ERFDKEiKbIhCeoAE9OEqj7mGIL6AHlIm0BFft4Da2EEIGGFoH6ltILBIi8YA3DEWg5u4XgsE3YxxER/AeWqbEsdy2Eqi8REpo9doptoPBImIb6PCwKaCHeFgIdIbFiEiFJMGlTA8eABnjBqpYqEMYBFF/P5/iB%2BFssq/hBBEMR2FBWRBCUVQNER3R9EMcWzEsawDEZubICWVBEk6LGAHpCVNqxLEMRFJgOB2gZUh2AHEACUAFpeETGJ3iwK3GTaDp0hcBh3E8Zo9ACBO%2BmKUoU9yNIBDGPx9Gzzp04GWJ9EqaoBG6UYk%2ByMv2gRGopmLuZS%2BGHo85TyYembzPsOWVZ1j0VKMsmxH8o4A47olDdFGOpSLtTa6DggXBCBII5QWw3hFq0BZ6sa5qJTalkuC2aQ4nuvq%2BoSNKOAm0hsty8fZvmqqJZWmBEBAFYCESKndogPtbaxAwisA2GobqcQg73RgsbYABwuAslTFIVMlUkRrwjinVWwhRDiC4AkbBut1BTUNqQf4zNEgQ2HhwTKD8prjyBlTP%2B35UDQSnjPOe3wF5LwgB4TawCN5bC3m/Jae9SANUQS1PqEpuoSikHEKQfV%2BosjGnfUeT8Zq2FfjvWqqiJDqNDponRYjeaoUakAA%3D
    return std::move(*expected);
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
    //ASSERT(packetRandomBlock.size() == randomBlock.size(), "Buffer size mismatch when calling memcpy");
    //size_t bytesToCopy = randomBlock.size() < packetRandomBlock.size() ? randomBlock.size() : packetRandomBlock.size();
    //std::memcpy(packetRandomBlock.data(), randomBlock.data(), bytesToCopy);
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
    //ASSERT(packetHashBlock.size() == hash.size(), "Buffer size mismatch when calling memcpy");
    //size_t bytesToCopy = hash.size() < packetHashBlock.size() ? hash.size() : packetHashBlock.size();
    //std::memcpy(packetHashBlock.data(), hash.data(), bytesToCopy);
    insert_data(packetHashBlock, std::span<const uint8_t>{ hash.data(), hash.size() });

    return packet;
}
[[nodiscard]] std::vector<byte> insert_signature(std::vector<byte>& packet, const std::vector<byte>& signature)
{
    static constexpr ble::DemandRSSIHeader HEADER = ble::header_whereami_demand_rssi();

    packet[HEADER.signatureOffset] = sizeof(decltype(HEADER)) + packet[HEADER.randomDataSize] + packet[HEADER.hashSize];
    packet[HEADER.signatureSize] = common::assert_down_cast<uint8_t>(signature.size());

    std::span<uint8_t> packetSignatureBlock{ std::begin(packet) + packet[HEADER.signatureOffset], packet[HEADER.signatureSize] };
    //ASSERT(packetSignatureBlock.size() == signature.size(), "Buffer size mismatch when calling memcpy");
    //size_t bytesToCopy = signature.size() < packetSignatureBlock.size() ? signature.size() : packetSignatureBlock.size();
    //std::memcpy(packetSignatureBlock.data(), signature.data(), bytesToCopy);
    insert_data(packetSignatureBlock, signature);

    return packet;
}
[[nodiscard]] std::vector<byte> make_packet_demand_rssi(security::CEccPrivateKey* pClientKey)
{
    using sha_type = security::Sha2_256;
    static std::expected<security::CRandom, security::CRandom::Error> expected = security::CRandom::make_rng();
    if (!expected)
    {
        LOG_FATAL("Failed to create cryptographic rng generator");
    }

    security::CRandom& rng = *expected;
    std::vector<byte> randomBlock = generate_random_block(rng);
    security::CHash<sha_type> hash{ randomBlock };
    std::vector<byte> signature = pClientKey->sign_hash(rng, hash);

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
}    // namespace
CRssiDemander::CRssiDemander(CServer& server, gfx::CWindow& window, std::chrono::seconds demandInterval)
    : m_Queue{}
    , m_DemandInterval{ demandInterval }
    , m_pServerPubKey{ load_key<security::CEccPublicKey>(SERVER_PUBLIC_KEY_NAME) }
    , m_pClientPrivKey{ load_key<security::CEccPrivateKey>(CLIENT_PRIVATE_KEY_NAME) }
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
                    pSelf->m_Queue.push(rssi);
                }
                else
                {
                    // TODO:: missed answers counter
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
                //gfx::CWindow& window = *(pSelf->m_pWindow);
                //pSelf->m_pServer->demand_rssi(window);
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

        co_await pSelf->try_demand_rssi(wpCharacteristic.value(), pSelf->make_packet_demand_rssi());
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
    std::vector<byte> randomBlock = generate_random_block(rng);
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
