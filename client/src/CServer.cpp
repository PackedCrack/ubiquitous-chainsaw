//
// Created by qwerty on 2024-04-25.
//
#include "CServer.hpp"
#include "client_common.hpp"
#include "common/CCoroutineManager.hpp"
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
    ASSERT(expected, "Generating random data should never fail");

    // This is NOT a bug
    // https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QAJgAMfAQQCqAZ0wAFAB6cpvAFYgupWkwahkAUgkAhcxdLL6yAngGVG6AMKpaAVxYNJpVwAZPAZMADkfACNMYkkANlIAB1RFQicGD29ff2TUxwFg0IiWaNiJBLtMB3ShAiZiAkyfPwlbTHt8hlr6gkLwqJj42zqGpuzWxRHekP6SwfKASltUL2Jkdg5zAGYQ5G8sAGpTLbdkSfxBY%2BxTKQBBbd39zCOTyeIQ4AB9ADc8TAB3K43e4SHYMPZeQ7HNyYNSJKoETDoIF3B7gp4vNzfBEkFH3O7nEAgbEOXEnZAIepXA7ATAELgQBbA0wAdisdwOnIOxDpqwYRzZ%2BgOrQOW1IBwALKyACLHdn3Fmy1EEgjoIkkojEaEUqlbbA0ukSRnMtnArkHQkgWHwhxI6GWjVk06UrV68UhAjU60I1lWQXikViyUykNbeVmrk8gh8i2qoksVDYiDehwAOm%2BYi8mEZTLDJqV%2BNuDpxruduv1tIIW2NqNNHK5lpTiORrzjxJL2pdV3dlz1Bybvt9QsD4qlitD4frnKjMab6cz2dz4fHytuHoOLCYIQg6/qwGQ4p1xAAVMeDnvvkza/LzWgGJNrcRY2qQKkAF6YT4EA0EABqMVSAQXmlYU5WZKcDjwKgDjvB84WIaA6X/YhAP5Y5ZS2ECuCvW5zUHCDzWLUlSyPakSWAn8GSXCNzWnXliH5ElU3fRcwIgkMIPaZRIOg2DEXgxC/wA9JgPQ4UcLwutcNozkiM1TtywOEkJAoysjWogjI3oxiqgkZi8A/Y083YldpM5LjnigmCBDgxIEMrZDUNEzDRQkrl8LM2i5KdUi%2BxJLZVLpasNM880ZwYpSqi2fTDJCySCzw2tpQ4JZaE4ABWXg/A4LRSFQTg3GsawLRWNZnm2HhSAITQUqWABrEAJS4VMJHSiQJQlOIuAADi2ABOdKtikdL0oMTgJSymq8s4XhFBAGRqpylLSDgWAkDQFhEjoGJyEoDatvoWJvmQRJEh%2BLg%2Bs%2BLZus%2BNQ4glPg6ERFDKEiKbIhCeoAE9OEqj7mGIL6AHlIm0BFft4Da2EEIGGFoH6ltILBIi8YA3DEWg5u4XgsE3YxxER/AeWqbEsdy2Eqi8REpo9doptoPBImIb6PCwKaCHeFgIdIbFiEiFJMGlTA8eABnjBqpYqEMYBFF/P5/iB%2BFssq/hBBEMR2FBWRBCUVQNER3R9EMcWzEsawDEZubICWVBEk6LGAHpCVNqxLEMRFJgOB2gZUh2AHEACUAFpeETGJ3iwK3GTaDp0hcBh3E8Zo9ACBO%2BmKUoU9yNIBDGPx9Gzzp04GWJ9EqaoBG6UYk%2ByMv2gRGopmLuZS%2BGHo85TyYembzPsOWVZ1j0VKMsmxH8o4A47olDdFGOpSLtTa6DggXBCBII5QWw3hFq0BZ6sa5qJTalkuC2aQ4nuvq%2BoSNKOAm0hsty8fZvmqqJZWmBEBAFYCESKndogPtbaxAwisA2GobqcQg73RgsbYABwuAslTFIVMlUkRrwjinVWwhRDiC4AkbBut1BTUNqQf4zNEgQ2HhwTKD8prjyBlTP%2B35UDQSnjPOe3wF5LwgB4TawCN5bC3m/Jae9SANUQS1PqEpuoSikHEKQfV%2BosjGnfUeT8Zq2FfjvWqqiJDqNDponRYjeaoUakAA%3D
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
void CServer::subscribe(std::function<void(std::span<uint8_t>)>&& cb)
{
    auto& coroutineManager = common::coroutine_manager_instance();
    auto coroutine = [](std::weak_ptr<ble::CDevice> wpDevice, std::function<void(std::span<uint8_t>)> cb) -> sys::awaitable_t<void>
    {
        std::shared_ptr<ble::CDevice> pDevice = wpDevice.lock();
        if (!pDevice)
        {
            co_return;
        }

        std::optional<std::weak_ptr<ble::CCharacteristic>> wpCharacteristic =
            pDevice->characteristic(ble::uuid_service_whereami(), ble::uuid_characteristic_whereami_rssi_notification());
        if (!wpCharacteristic)
        {
            LOG_ERROR("Could not find Service: \"WhereAmI\" or Characteristic: \"RSSI Notification\" when trying to subscribe.");
            co_return;
        }

        std::shared_ptr<ble::CCharacteristic> pCharacteristic = wpCharacteristic->lock();
        if (pCharacteristic)
        {
            ble::CharacteristicSubscriptionState state = co_await pCharacteristic->subscribe_to_notify(std::move(cb));
            UNHANDLED_CASE_PROTECTION_ON
            switch (state)
            {
            case ble::CharacteristicSubscriptionState::notSubscribed:
            {
                LOG_ERROR("Failed to subscribe to Characteristic: \"RSSI Notification\".");
                break;
            }
            case ble::CharacteristicSubscriptionState::inFlight:
            {
#ifndef NDEUBG
                LOG_INFO("Subscribe attempt to Characteristic: \"RSSI Notification\" was skipped because an attempt is alread in flight.");
#endif
                break;
            }
            case ble::CharacteristicSubscriptionState::subscribed:
            {
#ifndef NDEUBG
                LOG_INFO("Subscribed successfully to Characteristic: \"RSSI Notification\".");
#endif
            }
            }
            UNHANDLED_CASE_PROTECTION_OFF
        }
    };

    std::lock_guard lock{ *m_pMutex };
    if (m_Server)
    {
        coroutineManager.fire_and_forget(coroutine, m_Server->pDevice, std::move(cb));
    }
};
void CServer::unsubscribe()
{
    auto& coroutineManager = common::coroutine_manager_instance();
    auto coroutine = [](std::weak_ptr<ble::CDevice> wpDevice) -> sys::awaitable_t<void>
    {
        std::shared_ptr<ble::CDevice> pDevice = wpDevice.lock();
        if (!pDevice)
        {
            co_return;
        }

        std::optional<std::weak_ptr<ble::CCharacteristic>> wpCharacteristic =
            pDevice->characteristic(ble::uuid_service_whereami(), ble::uuid_characteristic_whereami_rssi_notification());
        if (!wpCharacteristic)
        {
            LOG_ERROR("Could not find Service: \"WhereAmI\" or Characteristic: \"RSSI Notification\" when trying to unsubscribe.");
            co_return;
        }

        std::shared_ptr<ble::CCharacteristic> pCharacteristic = wpCharacteristic->lock();
        if (pCharacteristic)
        {
            ble::CharacteristicSubscriptionState state = co_await pCharacteristic->unsubscribe();
            UNHANDLED_CASE_PROTECTION_ON
            switch (state)
            {
            case ble::CharacteristicSubscriptionState::subscribed:
            {
                LOG_ERROR("Failed to unsubscribe from Characteristic: \"RSSI Notification\".");
                break;
            }
            case ble::CharacteristicSubscriptionState::inFlight:
            {
#ifndef NDEUBG
                LOG_INFO(
                    "Unsubscribe attempt to Characteristic: \"RSSI Notification\" was skipped because an attempt is alread in flight.");
#endif
                break;
            }
            case ble::CharacteristicSubscriptionState::notSubscribed:
            {
#ifndef NDEUBG
                LOG_INFO("Unsubscribed successfully from Characteristic: \"RSSI Notification\".");
#endif
            }
            }
            UNHANDLED_CASE_PROTECTION_OFF
        }
    };

    std::lock_guard lock{ *m_pMutex };
    if (m_Server)
    {
        coroutineManager.fire_and_forget(coroutine, m_Server->pDevice);
    }
}
void CServer::demand_rssi(gfx::CWindow& window)
{
    auto& coroutineManager = common::coroutine_manager_instance();
    auto coroutine = [](CServer* pServer, std::weak_ptr<ble::CDevice> wpDevice, security::CEccPrivateKey* pClientKey, gfx::CWindow* pWindow)
        -> sys::awaitable_t<void>
    {
        std::shared_ptr<ble::CDevice> pDevice = wpDevice.lock();
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

        co_await pServer->try_demand_rssi(pWindow, wpCharacteristic.value(), make_packet_demand_rssi(pClientKey));
    };

    std::lock_guard lock{ *m_pMutex };
    if (m_Server)
    {
        coroutineManager.fire_and_forget(coroutine, this, m_Server->pDevice, m_pClientPrivateKey.get(), &window);
    }
}
bool CServer::connected() const
{
    std::lock_guard lock{ *m_pMutex };
    if (m_Server)
    {
        return m_Server->pDevice->connected();
    }

    return false;
}
bool CServer::is_authenticated() const
{
    std::lock_guard lock{ *m_pMutex };
    return m_Server.has_value();
}
sys::awaitable_t<CServer::HasSubscribedResult> CServer::has_subscribed() const
{
    // This is a coroutine and it should have its own shared_ptr to CDevice
    // in case the Server gets deauthenticated from another thread
    std::shared_ptr<ble::CDevice> pDevice{};
    {
        std::lock_guard lock{ *m_pMutex };
        if (m_Server)
        {
            pDevice = m_Server->pDevice;
        }
        else
        {
            co_return HasSubscribedResult::notAuthenticated;
        }
    }

    ASSERT(pDevice, "Should always be valid if we get here");

    std::optional<std::weak_ptr<ble::CCharacteristic>> wpCharacteristic =
        pDevice->characteristic(ble::uuid_service_whereami(), ble::uuid_characteristic_whereami_rssi_notification());
    ASSERT(wpCharacteristic, "This characteristic should always exist");

    std::shared_ptr<ble::CCharacteristic> pCharacteristic = wpCharacteristic->lock();
    if (pCharacteristic)
    {
        ble::CharacteristicSubscriptionState state = co_await pCharacteristic->has_subscribed();
        // clang-format off
        UNHANDLED_CASE_PROTECTION_ON
        switch (state)
        {
            case ble::CharacteristicSubscriptionState::subscribed: co_return HasSubscribedResult::subscribed; 
            case ble::CharacteristicSubscriptionState::notSubscribed: co_return HasSubscribedResult::notSubscribed;
            case ble::CharacteristicSubscriptionState::inFlight: co_return HasSubscribedResult::inFlight;
        }
        UNHANDLED_CASE_PROTECTION_OFF
        // clang-format off
    }

    // If we fail to take ownership of the needed characteristic.. Just return notAuthenticated.
    co_return HasSubscribedResult::notAuthenticated;
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
sys::awaitable_t<void>
    CServer::try_demand_rssi(gfx::CWindow* pWindow, std::weak_ptr<ble::CCharacteristic> characteristic, std::vector<byte> packet)
{
    const std::shared_ptr<ble::CCharacteristic>& pCharacteristic = characteristic.lock();

    static constexpr int32_t MAX_ATTEMPS = 1;
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
            pWindow->popup_warning("Unreachable", "Could not demand RSSI value from server");
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
            pWindow->popup_warning("Access Denied", "Could not demand RSSI value from server");
            [[fallthrough]];
        }
        case ble::CommunicationStatus::protocolError:
        {
            ++attempt;
            LOG_WARN("Could not write to WhereAmI's demand RSSI characteristic - Protocol Error");
            pWindow->popup_warning("Protocol Error", "Could not demand RSSI value from server");
        }
        }
        UNHANDLED_CASE_PROTECTION_OFF
    } while (attempt < MAX_ATTEMPS);
}
