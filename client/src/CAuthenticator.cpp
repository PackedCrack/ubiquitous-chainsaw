#include "CAuthenticator.hpp"
#include "security/CHash.hpp"
#include "security/sha.hpp"
#include "bluetoothLE/Device.hpp"


namespace
{
using namespace security;
using ShaHash = std::variant<CHash<Sha2_224>, CHash<Sha2_256>, CHash<Sha3_224>, CHash<Sha3_256>, CHash<Sha3_384>, CHash<Sha3_512>>;
[[nodiscard]] ShaHash make_hash(ble::HashType type, std::span<uint8_t> hashData)
{
    UNHANDLED_CASE_PROTECTION_ON
    switch (type)
    {
        case ble::HashType::Sha2_224: return CHash<Sha2_224>{ hashData };
        case ble::HashType::Sha2_256: return CHash<Sha2_256>{ hashData };
        case ble::HashType::Sha3_224: return CHash<Sha3_224>{ hashData };
        case ble::HashType::Sha3_256: return CHash<Sha3_256>{ hashData };
        case ble::HashType::Sha3_384: return CHash<Sha3_384>{ hashData };
        case ble::HashType::Sha3_512: return CHash<Sha3_512>{ hashData };
        case ble::HashType::count:
            LOG_FATAL("Value of \"count\" passed unexpectedly from ble::HashType");
    }
    UNHANDLED_CASE_PROTECTION_OFF
    
    std::unreachable();
}
[[nodiscard]] sys::awaitable_t<std::optional<const ble::CCharacteristic*>> find_server_auth_characteristic(const ble::CDevice& device)
{
    std::optional<const ble::CService*> service = device.service(ble::uuid_service_whoami());
    if (!service)
        co_return std::nullopt;
    
    std::optional<const ble::CCharacteristic*> characteristic = service.value()->characteristic(ble::uuid_characteristic_server_auth());
    if (!characteristic)
        co_return std::nullopt;
    
    co_return characteristic;
}
[[nodiscard]] bool buffer_size_mismatch(auto&& buffer)
{
    static constexpr ble::ServerAuthHeader HEADER = ble::whoami_server_auth_header();
    static constexpr size_t expectedBufferSize = sizeof(HEADER) + HEADER.hashSize + HEADER.signatureSize;
    if (expectedBufferSize != buffer.size())
    {
        LOG_ERROR_FMT("Buffer size mismatch. Expected {} bytes, but buffer was {}", expectedBufferSize, buffer.size());
        return true;
    }
    
    return false;
}
[[nodiscard]] bool valid_hash_type(auto&& buffer)
{
    static constexpr ble::ServerAuthHeader HEADER = ble::whoami_server_auth_header();
    uint8_t hashType = buffer[HEADER.hashType];
    if (hashType < std::to_underlying(ble::HashType::count))
        return true;
    
    LOG_ERROR_FMT("Unknown hashtype value in packet's server auth header: \"{}\"", hashType);
    return false;
}
[[nodiscard]] sys::awaitable_t<bool> verify_hash_and_signature(
        std::string_view macAddress,
        Pointer<security::CEccPublicKey> key,
        ShaHash& hash,
        std::span<uint8_t> signature)
{
    bool result{};
    std::visit(
            [&result, &signature, macAddress, key](auto&& hash)
            {
                using sha_type = typename std::remove_cvref_t<decltype(hash)>::hash_type;
                security::CHash<sha_type> macHash{ macAddress };
                if (hash.size() != sha_type::HASH_SIZE)
                {
                    result = false;
                }
                else if (macHash != hash)
                {
                    result = false;
                }
                else
                {
                    result = key->verify_hash(signature, hash);
                }
            },
            hash);
    
    co_return result;
}
}   // namespace


CAuthenticator::CAuthenticator(security::CEccPublicKey* pServerKey)
    : m_pServerKey{ pServerKey }
    , m_Devices{}
    , m_ServerInfo{ std::nullopt }
    , m_pSharedMutex{ std::make_unique<std::shared_mutex>() }
{}
CAuthenticator::CAuthenticator(const CAuthenticator& other)
        : m_pServerKey{ nullptr }
        , m_Devices{}
        , m_ServerInfo{ std::nullopt }
        , m_pSharedMutex{ nullptr }
{
    copy(other);
}
CAuthenticator& CAuthenticator::operator=(const CAuthenticator& other)
{
    if(this != &other)
        copy(other);
    
    return *this;
}
void CAuthenticator::copy(const CAuthenticator& other)
{
    m_pServerKey = other.m_pServerKey;
    m_Devices = other.m_Devices;
    m_ServerInfo = other.m_ServerInfo;
    m_pSharedMutex =  std::make_unique<std::shared_mutex>();
}
void CAuthenticator::enqueue_devices(const std::vector<ble::DeviceInfo>& infos)
{
    {
        std::lock_guard lock{ *m_pSharedMutex };
        for (auto&& info : infos)
            m_Devices.push_back(info);
    }

    process_queue();
}
[[nodiscard]] bool CAuthenticator::server_identified() const
{
    std::lock_guard lock{ *m_pSharedMutex };
    return m_ServerInfo.has_value();
}
[[nodiscard]] std::string CAuthenticator::server_address() const
{
    std::lock_guard lock{ *m_pSharedMutex };
    return ble::DeviceInfo::address_as_str(m_ServerInfo->address.value());
}
sys::fire_and_forget_t CAuthenticator::process_queue()
{
    while(!m_Devices.empty())
    {
        ble::DeviceInfo info{};
        {
            std::lock_guard lock{ *m_pSharedMutex };
            info = m_Devices.front();
            m_Devices.pop_front();
        }
        
        if (bool verified = co_await verify_server_address(info); verified)
        {
            std::lock_guard lock{ *m_pSharedMutex };
            m_ServerInfo.emplace(info);
        }
    }
}
sys::awaitable_t<bool> CAuthenticator::verify_server_address(ble::DeviceInfo info) const
{
    if (!info.address)
        co_return false;
    
    std::expected<ble::CDevice, ble::CDevice::Error> expected = co_await ble::make_device<ble::CDevice>(info.address.value());
    if(!expected)
        co_return false;
    
    std::optional<const ble::CCharacteristic*> characteristic = co_await find_server_auth_characteristic(expected.value());
    if (!characteristic)
        co_return false;
    
    
    const ble::CCharacteristic* pCharacteristic = characteristic.value();
    static constexpr int32_t MAX_ATTEMPTS = 3u;
    int32_t attempt{ 0 };
    do
    {
        ble::CCharacteristic::read_t result = co_await pCharacteristic->read_value();
        if (result)
        {
            auto&& buffer = result.value();
            if (buffer_size_mismatch(buffer))
                co_return false;
            if (!valid_hash_type(buffer))
                co_return false;
            
            
            static constexpr ble::ServerAuthHeader HEADER = ble::whoami_server_auth_header();
            auto hashType = ble::HashType{ buffer[HEADER.hashType] };
            ShaHash hash = make_hash(hashType, std::span<uint8_t>{ std::begin(buffer) + HEADER.hashOffset, HEADER.hashSize });
            
            std::span<uint8_t> signature{ std::begin(buffer) + HEADER.signatureOffset, HEADER.signatureSize };
            std::string macAddress = ble::DeviceInfo::address_as_str(info.address.value());
            
            co_return co_await verify_hash_and_signature(macAddress, m_pServerKey, hash, signature);
        }
        else
        {
            LOG_WARN_FMT("Attempt {} of {} failed to read value from Characteristic: {:#X}. Reason: {}",
                         attempt,
                         MAX_ATTEMPTS,
                         ble::ID_CHARACTERISTIC_SERVER_AUTH,
                         ble::gatt_communication_status_to_str(result.error()));
            ++attempt;
        }
        
    } while (attempt < MAX_ATTEMPTS);
    
    LOG_ERROR_FMT("Failed to read value for server auth characteristic with UUID: \"{:#X}\"",
                  ble::ID_CHARACTERISTIC_SERVER_AUTH);
    co_return false;
}