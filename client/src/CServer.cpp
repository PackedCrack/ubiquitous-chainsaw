//
// Created by qwerty on 2024-04-25.
//
#include "CServer.hpp"
#include "common/client_common.hpp"
#include "common/CCoroutineManager.hpp"
//
//
//
//
namespace
{
template<typename buffer_t>
requires common::const_buffer<buffer_t>
[[nodiscard]] bool buffer_size_mismatch(buffer_t&& buffer)
{
    static constexpr ble::AuthenticateHeader HEADER = ble::header_whoami_authenticate();
    const size_t EXPECTED_BUFFER_SIZE =
        sizeof(HEADER) + ble::size_of_hash_type(ble::ShaVersion{ buffer[HEADER.shaVersion] }) + buffer[HEADER.signatureSize];
    if (EXPECTED_BUFFER_SIZE != buffer.size())
    {
        LOG_ERROR_FMT("Buffer size mismatch. Expected {} bytes, but buffer was {}", EXPECTED_BUFFER_SIZE, buffer.size());
        return true;
    }

    return false;
}
template<typename buffer_t>
requires common::const_buffer<buffer_t>
[[nodiscard]] bool valid_hash_type(buffer_t&& buffer)
{
    static constexpr ble::AuthenticateHeader HEADER = ble::header_whoami_authenticate();
    uint8_t shaVersion = buffer[HEADER.shaVersion];
    if (shaVersion < std::to_underlying(ble::ShaVersion::count))
    {
        return true;
    }

    LOG_ERROR_FMT("Unknown sha version value in packet's server auth header: \"{}\"", shaVersion);
    return false;
}
[[nodiscard]] sys::awaitable_t<std::optional<std::weak_ptr<ble::CCharacteristic>>>
    find_characteristic_authenticate(const std::shared_ptr<ble::CDevice>& pDevice)
{
    std::optional<std::weak_ptr<ble::CService>> service = pDevice->service(ble::uuid_service_whoami());
    if (service)
    {
        std::shared_ptr<ble::CService> pService = service->lock();
        if (pService)
        {
            co_return pService->characteristic(ble::uuid_characteristic_whoami_authenticate());
        }
    }

    co_return std::nullopt;
}
[[nodiscard]] sys::awaitable_t<bool>
    verify_hash_and_signature(std::string_view macAddress, security::CEccPublicKey* pKey, ble::ShaHash& hash, std::span<uint8_t> signature)
{
    bool result{};
    std::visit(
        [&result, &signature, macAddress, pKey](auto&& hash)
        {
            using sha_type = typename std::remove_cvref_t<decltype(hash)>::hash_type;
            security::CHash<sha_type> macHash{ macAddress };
            if (hash.size() != sha_type::HASH_SIZE)
            {
                LOG_ERROR_FMT("Hash size mismatch. Received: {}. Self created: {}", hash.size(), sha_type::HASH_SIZE);
                result = false;
            }
            else if (macHash != hash)
            {
                LOG_ERROR_FMT("Self created hash of mac address is not the same as received hashed. Recived: {}. Self created: {}",
                              macHash.as_string(),
                              hash.as_string());
                result = false;
            }
            else
            {
                result = pKey->verify_hash(signature, hash);
            }
        },
        hash);

    co_return result;
}
};    // namespace
CServer::CServer()
    : m_pMutex{ std::make_unique<mutex_type>() }
    , m_Server{}
    , m_pServerKey{ load_key<security::CEccPublicKey>(SERVER_PUBLIC_KEY_NAME) }
    , m_Devices{}
{}
CServer::CServer(const CServer& other)
    : m_pMutex{ std::make_unique<mutex_type>() }
    , m_Server{}
    , m_pServerKey{ load_key<security::CEccPublicKey>(SERVER_PUBLIC_KEY_NAME) }
    , m_Devices{}
{
    std::lock_guard lock{ *other.m_pMutex };
    m_Server = other.m_Server;
    m_Devices = other.m_Devices;
}
CServer& CServer::operator=(const CServer& other)
{
    if (this != &other)
    {
        std::lock_guard lock{ *other.m_pMutex };

        m_pMutex = std::make_unique<mutex_type>();
        m_Server = other.m_Server;
        m_pServerKey = load_key<security::CEccPublicKey>(SERVER_PUBLIC_KEY_NAME);
        m_Devices = other.m_Devices;
    }

    return *this;
}
void CServer::enqueue_devices(const std::vector<ble::DeviceInfo>& infos)
{
    m_Devices.push(infos);
    process_queue();
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
bool CServer::reload_public_key()
{
    std::lock_guard lock{ *m_pMutex };
    m_pServerKey = load_key<security::CEccPublicKey>(SERVER_PUBLIC_KEY_NAME);
    return m_pServerKey ? true : false;
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
std::optional<std::shared_ptr<ble::CDevice>> CServer::device()
{
    std::lock_guard lock{ *m_pMutex };
    return m_Server.has_value() ? std::make_optional<std::shared_ptr<ble::CDevice>>(m_Server->pDevice) : std::nullopt;
}
auto CServer::make_connection_changed_cb()
{
    return [pSelf = this](ble::ConnectionStatus status)
    {
        switch (status)
        {
            case ble::ConnectionStatus::connected:
                LOG_INFO("Connection status changed to connected?");
                break;
            case ble::ConnectionStatus::disconnected:
                pSelf->revoke_authentication();
                LOG_WARN("Lost connection to authenticated server...");
                break;
        }
    };
}
void CServer::process_queue()
{
    common::CCoroutineManager& manager = common::coroutine_manager_instance();
    auto coroutine = [](CServer* pSelf, ble::DeviceInfo info) -> sys::awaitable_t<void>
    {
        if (!info.address)
        {
            co_return;
        }

        std::expected<std::shared_ptr<ble::CDevice>, ble::CDevice::Error> expectedDevice =
            co_await ble::make_device<ble::CDevice>(info.address.value(), pSelf->make_connection_changed_cb());


        if (expectedDevice)
        {
            if (bool verified = co_await pSelf->verify_server_address(*expectedDevice, info.address.value()); verified)
            {
                ASSERT(pSelf, "Pointer to server should be valid for the duration of the program");
                if (pSelf->is_authenticated())
                {
                    co_return;
                }

                pSelf->grant_authentication(AuthenticatedDevice{ .pDevice = std::move(*expectedDevice), .info = info });
            }
        }
    };

    while (!m_Devices.empty())
    {
        ble::DeviceInfo info{};
        m_Devices.pop(info);

        manager.fire_and_forget(coroutine, this, info);
    }
}
sys::awaitable_t<bool> CServer::verify_server_address(const std::shared_ptr<ble::CDevice>& pDevice, uint64_t address) const
{
    std::optional<std::weak_ptr<ble::CCharacteristic>> characteristic = co_await find_characteristic_authenticate(pDevice);
    if (!characteristic)
    {
        co_return false;
    }


    std::shared_ptr<ble::CCharacteristic> pCharacteristic = characteristic->lock();
    static constexpr int32_t MAX_ATTEMPTS = 3u;
    int32_t attempt{ 0 };
    do
    {
        ble::CCharacteristic::read_t result = co_await pCharacteristic->read_value();
        if (result)
        {
            auto&& buffer = result.value();
            if (buffer_size_mismatch(buffer))
            {
                co_return false;
            }
            if (!valid_hash_type(buffer))
            {
                co_return false;
            }

            static constexpr ble::AuthenticateHeader HEADER = ble::header_whoami_authenticate();
            auto shaVersion = ble::ShaVersion{ buffer[HEADER.shaVersion] };

            const uint8_t HASH_OFFSET = buffer[HEADER.hashOffset];
            const uint8_t HASH_SIZE = buffer[HEADER.hashSize];
            ble::ShaHash hash = make_sha_hash(shaVersion, std::span<uint8_t>{ std::begin(buffer) + HASH_OFFSET, HASH_SIZE });

            const uint8_t SIGNATURE_OFFSET = buffer[HEADER.signatureOffset];
            const uint8_t SIGNATURE_SIZE = buffer[HEADER.signatureSize];
            std::span<uint8_t> signature{ std::begin(buffer) + SIGNATURE_OFFSET, SIGNATURE_SIZE };

            ASSERT(m_pServerKey, "This should always be valid - key reload waits for coro to finnish");
            std::string macAddress = ble::DeviceInfo::address_as_str(address);
            co_return co_await verify_hash_and_signature(macAddress, m_pServerKey.get(), hash, signature);
        }
        else
        {
            LOG_WARN_FMT("Attempt {} of {} failed to read value from Characteristic: {:#X}. Reason: {}",
                attempt,
                MAX_ATTEMPTS,
                ble::ID_CHARACTERISTIC_WHOAMI_AUTHENTICATE,
                ble::communication_status_to_str(result.error()));
            ++attempt;
        }

    } while (attempt < MAX_ATTEMPTS);

    LOG_ERROR_FMT("Failed to read value for server auth characteristic with UUID: \"{:#X}\"", ble::ID_CHARACTERISTIC_WHOAMI_AUTHENTICATE);
    co_return false;
}