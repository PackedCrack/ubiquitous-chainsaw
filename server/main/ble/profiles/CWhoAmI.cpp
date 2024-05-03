#include "CWhoAmI.hpp"
#include "common/ble_services.hpp"
#include "CGattCharacteristic.hpp"
#include "../../server_common.hpp"
#include "../../shared/common/ble_services.hpp"
#include "../../shared/common/common.hpp"
// std
#include <cstdint>
#include <stdexcept>
#include <array>
#include <cstring>
#include <type_traits>
// clang-format off


// clang-format on
namespace
{
void copy_from_buffer(std::span<security::byte>& dst, std::span<const security::byte> src)
{
    ASSERT(dst.size() >= src.size(), "Destination buffer is too small to copy source buffer!");
    std::size_t smallest = dst.size() < src.size() ? dst.size() : src.size();
    std::memcpy(dst.data(), src.data(), smallest);
}
template<typename algorithm_t>
[[nodiscard]] std::vector<security::byte>
    copy_hash_and_signature(std::vector<security::byte>& packet, security::CHash<algorithm_t>& hash, const std::vector<byte>& signature)
{
    std::size_t neededSize = sizeof(ble::AuthenticateHeader) + hash.size();
    ASSERT(packet.size() >= neededSize, "Packet buffer is too small to insert the hash data!");
    std::span<security::byte> packetView{ std::begin(packet) + sizeof(ble::AuthenticateHeader), hash.size() };
    copy_from_buffer(packetView, std::span<const security::byte>{ hash.data(), hash.size() });

    neededSize = sizeof(ble::AuthenticateHeader) + hash.size() + signature.size();
    ASSERT(packet.size() >= neededSize, "Packet buffer is too small to insert the hash data!");
    packetView = std::span<security::byte>{ std::end(packetView), signature.size() };
    copy_from_buffer(packetView, std::span<const security::byte>{ signature.data(), signature.size() });

    return packet;
}
template<typename algorithm_t>
[[nodiscard]] std::vector<security::byte>
    fill_header(std::vector<security::byte>& packet, security::CHash<algorithm_t>& hash, const std::vector<byte>& signature)
{
    ASSERT(packet.size() > sizeof(ble::AuthenticateHeader), "Packet too small! Has it not been pre-allocated?");

    static constexpr ble::AuthenticateHeader HEADER{};
    packet[HEADER.shaVersion] = ble::sha_version_id(ble::sha_to_enum<algorithm_t>());
    packet[HEADER.hashOffset] = common::assert_down_cast<uint8_t>(sizeof(ble::AuthenticateHeader));
    packet[HEADER.hashSize] = common::assert_down_cast<uint8_t>(hash.size());
    packet[HEADER.signatureOffset] = common::assert_down_cast<uint8_t>(sizeof(ble::AuthenticateHeader) + hash.size());
    packet[HEADER.signatureSize] = common::assert_down_cast<uint8_t>(signature.size());

    return packet;
}
template<typename algorithm_t>
[[nodiscard]] std::vector<security::byte>
    make_packet(std::vector<security::byte>& packet, security::CHash<algorithm_t>& hash, const std::vector<byte>& signature)
{
    std::size_t packetSize = sizeof(ble::AuthenticateHeader) + hash.size() + signature.size();
    packet = std::vector<security::byte>(packetSize);
    ASSERT(packet.size() == packetSize, "Expected constructor to resize the new vector");

    packet = fill_header(packet, hash, signature);
    packet = copy_hash_and_signature(packet, hash, signature);

    return packet;
}
}    // namespace
namespace ble
{
CWhoAmI::CWhoAmI()
    : m_pPrivateKey{ load_key<security::CEccPrivateKey>(NVS_KEY_SERVER_PRIVATE) }
    , m_SignedMacData{}
    , m_Characteristics{}
    , m_Service{}
{}
CWhoAmI::CWhoAmI(const CWhoAmI& other)
    : m_pPrivateKey{ nullptr }
    , m_SignedMacData{}
    , m_Characteristics{}
    , m_Service{}
{
    copy(other);
}
CWhoAmI& CWhoAmI::operator=(const CWhoAmI& other)
{
    if (this != &other)
    {
        copy(other);
    }
    return *this;
}
void CWhoAmI::copy(const CWhoAmI& other)
{
    m_pPrivateKey = load_key<security::CEccPrivateKey>(NVS_KEY_SERVER_PRIVATE);
    m_SignedMacData = other.m_SignedMacData;
    m_Characteristics = other.m_Characteristics;
    m_Service = other.m_Service;
}
void CWhoAmI::register_with_nimble(const std::shared_ptr<Profile>& pProfile)
{
    m_Characteristics = make_characteristics(pProfile);
    m_Service = CGattService{ ID_SERVICE_WHOAMI, m_Characteristics };
}
void CWhoAmI::sign_server_mac_address()
{
    std::expected<std::string, ble::NimbleErrorCode> addressResult = ble::current_mac_address(ble::AddressType::randomMac);
    if (!addressResult)
    {
        LOG_FATAL_FMT("UNABLE TO RETRIEVE A MAC ADDRESS! ERROR = {}", nimble_error_to_string(addressResult.error()));
    }

    security::CRandom rng = security::CRandom::make_rng().value();
    security::CHash<security::Sha2_256> hash{ std::move(addressResult.value()) };
    std::vector<security::byte> signature = m_pPrivateKey->sign_hash(rng, hash);

    m_SignedMacData = make_packet(m_SignedMacData, hash, signature);
    ASSERT(!m_SignedMacData.empty(), "Failed to sign server MAC address!");
}
ble_gatt_svc_def CWhoAmI::as_nimble_service() const
{
    return static_cast<ble_gatt_svc_def>(m_Service);
}
std::vector<CCharacteristic> CWhoAmI::make_characteristics(const std::shared_ptr<Profile>& pProfile)
{
    std::vector<CCharacteristic> chars{};
    chars.emplace_back(make_characteristic_authenticate(pProfile));
    return chars;
}
auto CWhoAmI::make_callback_authenticate(const std::shared_ptr<Profile>& pProfile)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
    return [wpProfile =
                std::weak_ptr<Profile>{ pProfile }](uint16_t connectionHandle,
                                                    uint16_t attributeHandle,
                                                    ble_gatt_access_ctxt* pContext) -> int    // type deduction requires exact typematch
    {
        std::shared_ptr<Profile> pProfile = wpProfile.lock();
        if (pProfile)
        {
            CWhoAmI* pSelf = std::get_if<CWhoAmI>(pProfile.get());
            if (pSelf != nullptr)
            {
                auto operation = CharacteristicAccess{ pContext->op };
                UNHANDLED_CASE_PROTECTION_ON
                switch (operation)
                {
                case CharacteristicAccess::read:
                {
                    if (pSelf->m_SignedMacData.empty())
                    {
                        pSelf->sign_server_mac_address();
                    }

                    NimbleErrorCode code = append_read_data(pContext->om, pSelf->m_SignedMacData);
                    if (code != NimbleErrorCode::success)
                    {
                        LOG_ERROR_FMT("Characteristic callback for Server Auth failed to append its data to the client: \"{}\"",
                                      nimble_error_to_string(code));
                    }

                    return static_cast<int32_t>(code);
                }
                case CharacteristicAccess::write:
                {
                    ASSERT_FMT(false,
                               "Read only Characteristic \"Authenticate\" recieved a Write operation from connection handle: \"{}\"",
                               connectionHandle);
                }
                }
                UNHANDLED_CASE_PROTECTION_OFF
            }
            else
            {
                LOG_WARN("Characteristic callback for \"Authenticate\" failed to retrieve pointer to self from shared_ptr to Profile.");
            }
        }
        else
        {
            LOG_WARN(
                "Characteristic callback for \"Authenticate\" failed to take ownership of shared pointer to profile! It has been deleted.");
        }

        return static_cast<int32_t>(NimbleErrorCode::unexpectedCallbackBehavior);
    };
#pragma GCC diagnostic pop
}
CCharacteristic CWhoAmI::make_characteristic_authenticate(const std::shared_ptr<Profile>& pProfile)
{
    return make_characteristic(ID_CHARACTERISTIC_WHOAMI_AUTHENTICATE, make_callback_authenticate(pProfile), CharsPropertyFlag::read);
}
}    // namespace ble
