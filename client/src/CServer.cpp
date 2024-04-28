//
// Created by qwerty on 2024-04-25.
//
#include "CServer.hpp"
#include "client_common.hpp"
// clang-format off


// clang-format on
namespace
{
[[nodiscard]] std::vector<byte> generate_random_block(security::CRandom& rng)
{
    static std::random_device rd{};
    static std::mt19937_64 generator{ rd() };
    static std::uniform_int_distribution<size_t> distribution{ 64u, 96u };
    size_t blockSize = distribution(generator);

    std::expected<std::vector<byte>, security::CRandom::Error> expected = rng.generate_block(blockSize);
    if (!expected)
    {
        LOG_ERROR_FMT("Failed to generate random block of data of size: {}", blockSize);
        return { 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD };
    }

    return std::move(*expected);
}
[[nodiscard]] std::vector<byte> insert_random_data_block(std::vector<byte>& packet, const std::vector<byte>& randomBlock)
{
    static constexpr ble::DemandRSSIHeader HEADER = ble::header_whereami_demand_rssi();

    packet[HEADER.randomDataOffset] = sizeof(decltype(HEADER));
    packet[HEADER.randomDataSize] = common::assert_down_cast<uint8_t>(randomBlock.size());

    std::span<uint8_t> packetRandomBlock{ std::begin(packet) + packet[HEADER.randomDataOffset], packet[HEADER.randomDataSize] };
    ASSERT(packetRandomBlock.size() == randomBlock.size(), "Buffer size mismatch when calling memcpy");
    size_t bytesToCopy = randomBlock.size() < packetRandomBlock.size() ? randomBlock.size() : packetRandomBlock.size();
    std::memcpy(packetRandomBlock.data(), randomBlock.data(), bytesToCopy);

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
    ASSERT(packetHashBlock.size() == hash.size(), "Buffer size mismatch when calling memcpy");
    size_t bytesToCopy = hash.size() < packetHashBlock.size() ? hash.size() : packetHashBlock.size();
    std::memcpy(packetHashBlock.data(), hash.data(), bytesToCopy);

    return packet;
}
[[nodiscard]] std::vector<byte> insert_signature(std::vector<byte>& packet, const std::vector<byte>& signature)
{
    static constexpr ble::DemandRSSIHeader HEADER = ble::header_whereami_demand_rssi();

    packet[HEADER.signatureOffset] = sizeof(decltype(HEADER)) + packet[HEADER.randomDataSize] + packet[HEADER.hashSize];
    packet[HEADER.signatureSize] = common::assert_down_cast<uint8_t>(signature.size());

    std::span<uint8_t> packetSignatureBlock{ std::begin(packet) + packet[HEADER.signatureOffset], packet[HEADER.signatureSize] };
    ASSERT(packetSignatureBlock.size() == signature.size(), "Buffer size mismatch when calling memcpy");
    size_t bytesToCopy = signature.size() < packetSignatureBlock.size() ? signature.size() : packetSignatureBlock.size();
    std::memcpy(packetSignatureBlock.data(), signature.data(), bytesToCopy);

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
template<typename T>
[[nodiscard]] auto log_failed_lookup(std::string_view msg)
{
    return [msg]() -> std::optional<std::weak_ptr<T>>
    {
        LOG_WARN_FMT("{}", msg.data());
        return {};
    };
}
[[nodiscard]] std::optional<std::weak_ptr<ble::CCharacteristic>>
    find_characteristic_demand_rssi(const std::shared_ptr<ble::CService>& pService)
{
    return std::optional<std::weak_ptr<ble::CCharacteristic>>{ pService->characteristic(ble::uuid_characteristic_whereami_demand_rssi()) };
}
[[nodiscard]] std::optional<std::weak_ptr<ble::CCharacteristic>>
    find_characteristic_rssi_notification(const std::shared_ptr<ble::CService>& pService)
{
    return std::optional<std::weak_ptr<ble::CCharacteristic>>{ pService->characteristic(
        ble::uuid_characteristic_whereami_rssi_notification()) };
}
[[nodiscard]] std::optional<std::weak_ptr<ble::CService>> find_service_whereami(const ble::CDevice& device)
{
    return std::optional<std::weak_ptr<ble::CService>>{ device.service(ble::uuid_service_whereami()) };
}
template<typename owner_t>
[[nodiscard]] auto take_temp_ownership(std::shared_ptr<owner_t>& pOwner)
{
    return [&pOwner](const std::weak_ptr<owner_t>& service)
    {
        pOwner = service.lock();
        return std::optional<std::weak_ptr<owner_t>>{ service };
    };
}
[[nodiscard]] std::shared_ptr<ble::CCharacteristic> get_characteristic_rssi_notification(const std::shared_ptr<ble::CService>& pService,
                                                                                         std::string_view errorMsg)
{
    std::shared_ptr<ble::CCharacteristic> pCharacteristic = nullptr;
    // clang-format off
    find_characteristic_rssi_notification(pService)
    .and_then(take_temp_ownership(pCharacteristic))
    .or_else(log_failed_lookup<ble::CCharacteristic>(errorMsg));
    // clang-format on

    return pCharacteristic;
}
};    // namespace
CServer::CServer()
    : m_pMutex{ std::make_unique<mutex_type>() }
    , m_pClientPrivateKey{ load_key<security::CEccPrivateKey>(CLIENT_PRIVATE_KEY_NAME) }
    , m_Server{}
{}
CServer::CServer(const CServer& other)
    : m_pMutex{ std::make_unique<mutex_type>() }
    , m_pClientPrivateKey{ load_key<security::CEccPrivateKey>(CLIENT_PRIVATE_KEY_NAME) }
    , m_Server{}
{
    std::lock_guard lock{ *other.m_pMutex };
    m_Server = other.m_Server;
}
CServer& CServer::operator=(const CServer& other)
{
    if (this != &other)
    {
        std::lock_guard lock{ *other.m_pMutex };

        m_pMutex = std::make_unique<mutex_type>();
        m_pClientPrivateKey = load_key<security::CEccPrivateKey>(CLIENT_PRIVATE_KEY_NAME);
        m_Server = other.m_Server;
    }

    return *this;
}
void CServer::grant_authentication(AuthenticatedDevice&& device)
{
    std::lock_guard lock{ *m_pMutex };
    m_Server.emplace(std::move(device));
}
void CServer::revoke_authentication()
{
    std::lock_guard lock{ *m_pMutex };
    m_Server = std::nullopt;
}
sys::fire_and_forget_t CServer::subscribe(std::function<void(std::span<const uint8_t>)> cb)
{
    if (is_authenticated())
    {
        std::shared_ptr<ble::CService> pService =
            get_service_whereami("Server could not find service \"WhereAmI\" when trying to subscribe!");
        if (pService)
        {
            std::shared_ptr<ble::CCharacteristic> pCharacteristic = get_characteristic_rssi_notification(
                pService,
                "Server could not find characteristic \"RSSI Notification\" when trying to subscribe!");
            if (pCharacteristic)
            {
                ble::CommunicationStatus status = co_await pCharacteristic->subscribe_to_notify(std::move(cb));
                if (status != ble::CommunicationStatus::success)
                {
                    LOG_ERROR_FMT("Failed to subscribe to characteristic \"RSSI Notification\". Reason: \"{}\"",
                                  ble::communication_status_to_str(status));
                }
            }
        }
    }
};
sys::fire_and_forget_t CServer::unsubscribe()
{
    if (is_authenticated())
    {
        std::shared_ptr<ble::CService> pService =
            get_service_whereami("Server could not find service \"WhereAmI\" when trying to unsubscribe!");

        if (pService)
        {
            std::shared_ptr<ble::CCharacteristic> pCharacteristic = get_characteristic_rssi_notification(
                pService,
                "Server could not find characteristic \"RSSI Notification\" when trying to unsubscribe!");
            if (pCharacteristic)
            {
                ble::CommunicationStatus status = co_await pCharacteristic->unsubscribe();
                if (status != ble::CommunicationStatus::success)
                {
                    LOG_ERROR_FMT("Failed to unsubscribe from \"RSSI Notification\". Reason: {}",
                                  ble::communication_status_to_str(status).data());
                }
            }
        }
    }
}
sys::fire_and_forget_t CServer::try_demand_rssi(gfx::CWindow& window,
                                                const std::weak_ptr<ble::CCharacteristic>& characteristic,
                                                const std::vector<byte>& packet)
{
    const std::shared_ptr<ble::CCharacteristic>& pCharacteristic = characteristic.lock();

    static constexpr int32_t MAX_ATTEMPS = 3;
    int32_t attempt{};
    do
    {
        auto communicationStatus = co_await pCharacteristic->write_data(packet);

        LOG_INFO_FMT("Write status: {}", ble::communication_status_to_str(communicationStatus));
        UNHANDLED_CASE_PROTECTION_ON
        switch (communicationStatus)
        {
        case ble::CommunicationStatus::unreachable:
        {
            revoke_authentication();
            window.popup_warning("Unreachable", "Could not demand RSSI value from server");
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
            window.popup_warning("Access Denied", "Could not demand RSSI value from server");
            [[fallthrough]];
        }
        case ble::CommunicationStatus::protocolError:
        {
            ++attempt;
            LOG_WARN("Could not write to WhereAmI's demand RSSI characteristic - Protocol Error");
            window.popup_warning("Protocol Error", "Could not demand RSSI value from server");
        }
        }
        UNHANDLED_CASE_PROTECTION_OFF
    } while (attempt < MAX_ATTEMPS);
}
sys::fire_and_forget_t CServer::demand_rssi(gfx::CWindow& window)
{
    std::shared_ptr<ble::CService> pService =
        get_service_whereami("Server could not find service \"WhereAmI\" when trying to demand RSSI!");

    if (pService)
    {
        std::optional<std::weak_ptr<ble::CCharacteristic>> characteristic = find_characteristic_demand_rssi(pService).or_else(
            log_failed_lookup<ble::CCharacteristic>("Failed to find \"Demand RSSI\" characteristic when trying to demand rssi!"));

        if (characteristic)
        {
            std::vector<byte> packet = make_packet_demand_rssi(m_pClientPrivateKey.get());
            try_demand_rssi(window, *characteristic, packet);
        }
    }

    co_return;
}
bool CServer::connected() const
{
    std::lock_guard lock{ *m_pMutex };
    if (m_Server)
    {
        return m_Server->device.connected();
    }

    return false;
}
bool CServer::is_authenticated() const
{
    std::lock_guard lock{ *m_pMutex };
    return m_Server.has_value();
}
sys::awaitable_t<bool> CServer::has_subscribed() const
{
    std::shared_ptr<ble::CService> pService =
        get_service_whereami("Server could not find service \"WhereAmI\" when trying to figure out if already subscribed!");

    if (pService)
    {
        std::shared_ptr<ble::CCharacteristic> pCharacteristic = get_characteristic_rssi_notification(
            pService,
            "Server could not find characteristic \"RSSI Notification\" when trying to figure out if already subscribed!");
        if (pCharacteristic)
        {
            co_return co_await pCharacteristic->has_subscribed();
        }
    }

    co_return false;
}
uint64_t CServer::server_address() const
{
    std::lock_guard lock{ *m_pMutex };
    return m_Server->info.address.value();
}
std::string CServer::server_address_as_str() const
{
    std::lock_guard lock{ *m_pMutex };
    return ble::DeviceInfo::address_as_str(m_Server->info.address.value());
}
std::shared_ptr<ble::CService> CServer::get_service_whereami(std::string_view errorMsg) const
{
    std::lock_guard lock{ *m_pMutex };

    if (m_Server)
    {
        std::shared_ptr<ble::CService> pService = nullptr;
        // clang-format off
        find_service_whereami(m_Server->device)
                .and_then(take_temp_ownership(pService))
                .or_else(log_failed_lookup<ble::CService>(errorMsg));
        // clang-format on

        return pService;
    }

    return nullptr;
}
