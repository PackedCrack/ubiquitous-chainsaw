//
// Created by qwerty on 2024-04-25.
//
#include "CServer.hpp"
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
[[nodiscard]] std::vector<byte> make_packet_demand_rssi(security::CEccPrivateKey* pPrivateKey)
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
    std::vector<byte> signature = pPrivateKey->sign_hash(rng, hash);

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
sys::fire_and_forget_t try_demand_rssi(gfx::CWindow& wnd,
                                       //CAuthenticator& authenticator,
                                       const ble::CCharacteristic& characteristic,
                                       const std::vector<byte>& packet)    // async function - intentional copy
{
    static constexpr int32_t MAX_ATTEMPS = 3;
    int32_t attempt{};
    do
    {
        auto communicationStatus = co_await characteristic.write_data(packet);

        LOG_INFO_FMT("Write status: {}", ble::gatt_communication_status_to_str(communicationStatus));
        switch (communicationStatus)
        {
        case ble::CommunicationStatus::unreachable:
        {
            //authenticator.deauth();
            wnd.popup_warning("Unreachable", "Could not demand RSSI value from server");
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
            wnd.popup_warning("Access Denied", "Could not demand RSSI value from server");
            [[fallthrough]];
        }
        case ble::CommunicationStatus::protocolError:
        {
            ++attempt;
        }
        }
    } while (attempt < MAX_ATTEMPS);
}
[[nodiscard]] std::optional<const ble::CCharacteristic*> find_characteristic_demand_rssi(const ble::CService* pService)
{
    return std::optional<const ble::CCharacteristic*>{ pService->characteristic(ble::uuid_characteristic_whereami_demand_rssi()) };
}
[[nodiscard]] auto send_demand(gfx::CWindow& wnd, /*CAuthenticator& authenticator,*/ security::CEccPrivateKey* pPrivateKey)
{
    return [&wnd, /*&authenticator,*/ pPrivateKey](const ble::CCharacteristic* pCharacteristic)
    {
        std::vector<byte> packet = make_packet_demand_rssi(pPrivateKey);
        try_demand_rssi(wnd, /*authenticator,*/ *pCharacteristic, packet);

        return std::optional<const ble::CCharacteristic*>{ pCharacteristic };
    };
}
};    // namespace
CServer::CServer()
    : m_pMutex{ std::make_unique<mutex_type>() }
    , m_Server{}
{}
CServer::CServer(const CServer& other)
    : m_pMutex{ std::make_unique<mutex_type>() }
    , m_Server{ other.m_Server }
{}
CServer& CServer::operator=(const CServer& other)
{
    if (this != &other)
    {
        m_pMutex = std::make_unique<mutex_type>();
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
void CServer::demand_rssi(gfx::CWindow& window, /*CAuthenticator& authenticator,*/ security::CEccPrivateKey* pKey) const
{
    m_Server->device.service(ble::uuid_service_whereami())
        .and_then(find_characteristic_demand_rssi)
        .and_then(send_demand(window, /*authenticator,*/ pKey));
}
bool CServer::connected() const
{
    if (!is_authenticated())
    {
        return false;
    }

    return m_Server->device.connected();
}
bool CServer::is_authenticated() const
{
    std::lock_guard lock{ *m_pMutex };
    return m_Server.has_value();
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
