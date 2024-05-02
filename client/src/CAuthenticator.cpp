#include "CAuthenticator.hpp"
#include "security/CHash.hpp"
#include "security/sha.hpp"
#include "bluetoothLE/Device.hpp"
#include "common/common.hpp"
#include "client_common.hpp"
#include "common/CCoroutineManager.hpp"
// clang-format off


// clang-format on
namespace
{
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
}    // namespace
CAuthenticator::CAuthenticator(CServer& server)
    : m_pServerKey{ load_key<security::CEccPublicKey>(SERVER_PUBLIC_KEY_NAME) }
    , m_pServer{ &server }
    , m_Devices{}
{}
CAuthenticator::CAuthenticator(const CAuthenticator& other)
    : m_pServerKey{ load_key<security::CEccPublicKey>(SERVER_PUBLIC_KEY_NAME) }
    , m_pServer{ other.m_pServer }
    , m_Devices{ other.m_Devices }
{}
CAuthenticator& CAuthenticator::operator=(const CAuthenticator& other)
{
    if (this != &other)
    {
        m_pServerKey = load_key<security::CEccPublicKey>(SERVER_PUBLIC_KEY_NAME);
        m_pServer = other.m_pServer;
        m_Devices = other.m_Devices;
    }

    return *this;
}
void CAuthenticator::enqueue_devices(const std::vector<ble::DeviceInfo>& infos)
{
    m_Devices.push(infos);
    process_queue();
}
void CAuthenticator::deauth()
{
    m_pServer->revoke_authentication();
}
bool CAuthenticator::server_identified() const
{
    return m_pServer->is_authenticated();
}
uint64_t CAuthenticator::server_address() const
{
    return m_pServer->server_address();
}
std::string CAuthenticator::server_address_as_str() const
{
    return m_pServer->server_address_as_str();
}
auto CAuthenticator::make_connection_changed_cb() const
{
    return [pServer = m_pServer](ble::ConnectionStatus status)
    {
        switch (status)
        {
        case ble::ConnectionStatus::connected:
            LOG_INFO("Connection status changed to connected?");
            break;
        case ble::ConnectionStatus::disconnected:
            ASSERT(pServer, "Pointer to server should be valid for the duration of the program");
            pServer->revoke_authentication();
            LOG_WARN("Lost connection to authenticated server...");
            break;
        }
    };
}
void CAuthenticator::process_queue()
{
    common::CCoroutineManager& manager = common::coroutine_manager_instance();
    auto coroutine = [](CAuthenticator* pAuthenticator, Pointer<CServer> pServer, ble::DeviceInfo info) -> sys::awaitable_t<void>
    {
        if (!info.address)
        {
            co_return;
        }

        std::expected<std::shared_ptr<ble::CDevice>, ble::CDevice::Error> expectedDevice =
            co_await ble::make_device<ble::CDevice>(info.address.value(), pAuthenticator->make_connection_changed_cb());


        if (expectedDevice)
        {
            if (bool verified = co_await pAuthenticator->verify_server_address(*expectedDevice, info.address.value()); verified)
            {
                ASSERT(pServer, "Pointer to server should be valid for the duration of the program");
                if (pServer->is_authenticated())
                {
                    co_return;
                }

                pServer->grant_authentication(AuthenticatedDevice{ .pDevice = std::move(*expectedDevice), .info = info });
            }
        }
    };

    while (!m_Devices.empty())
    {
        ble::DeviceInfo info{};
        m_Devices.pop(info);

        manager.fire_and_forget(coroutine, this, m_pServer, info);
    }
}
sys::awaitable_t<bool> CAuthenticator::verify_server_address(const std::shared_ptr<ble::CDevice>& pDevice, uint64_t address) const
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
