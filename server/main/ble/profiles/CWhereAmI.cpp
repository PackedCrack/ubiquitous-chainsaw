#include "CWhereAmI.hpp"
#include "common/ble_services.hpp"
#include "CGattCharacteristic.hpp"
#include "../../shared/common/ble_services.hpp"
#include "../../shared/common/common.hpp"
#include "../../server_common.hpp"
#include "../ble_common.hpp"
// clang-format off


// clang-format on
namespace
{
void log_demand_rssi_header(const std::span<uint8_t> packet)
{
    static constexpr ble::DemandRSSIHeader HEADER{};
    LOG_INFO_FMT("randomDataOffset: {}. randomDataSize: {}. hashType: {}. hashOffset: {}. hashSize: {}. signatureOffset: "
                 "{}. signatureSize: {}",
                 packet[HEADER.randomDataOffset],
                 packet[HEADER.randomDataSize],
                 packet[HEADER.shaVersion],
                 packet[HEADER.hashOffset],
                 packet[HEADER.hashSize],
                 packet[HEADER.signatureOffset],
                 packet[HEADER.signatureSize]);
}
void log_rssi_notication_header(const std::span<uint8_t> packet)
{
    static constexpr ble::RSSINotificationHeader HEADER{};
    LOG_INFO_FMT("randomDataOffset: {}. randomDataSize: {}. rssiOffset: {}. rssiSize: {}. hashType: {}. hashOffset: {}. hashSize: {}. "
                 "signatureOffset: "
                 "{}. signatureSize: {}",
                 packet[HEADER.randomDataOffset],
                 packet[HEADER.randomDataSize],
                 packet[HEADER.rssiOffset],
                 packet[HEADER.rssiSize],
                 packet[HEADER.shaVersion],
                 packet[HEADER.hashOffset],
                 packet[HEADER.hashSize],
                 packet[HEADER.signatureOffset],
                 packet[HEADER.signatureSize]);
}
[[nodiscard]] security::CRandom& get_rng()
{
    // This should never fail so dont bother checking expected value. If it fails we can't continue the program anyway
    static security::CRandom rng = security::CRandom::make_rng().value();
    return rng;
}
[[nodiscard]] ble::NimbleErrorCode valid_context_buffer(const ble_gatt_access_ctxt* pContext)
{
    if (pContext->om == nullptr)
    {
        LOG_ERROR("WhereAmI's characteristic Demand RSSI recieved nullptr but packet was expected!");
        return ble::NimbleErrorCode::invalidArguments;
    }
    if (pContext->om->om_len < 1)
    {
        LOG_ERROR("WhereAmI's characteristic Demand RSSI recieved an empty packet!");
        return ble::NimbleErrorCode::toSmallBuffer;
    }

    return ble::NimbleErrorCode::success;
}
[[nodiscard]] bool valid_sha_version_id(ble::ShaVersion version)
{
    if (ble::sha_version_id(version) < ble::sha_version_id(ble::ShaVersion::count))
    {
        return true;
    }

    LOG_WARN_FMT("WhereAmI's characteristic DemandRSSI recieved a packet with an invalid ShaVersion: {}", ble::sha_version_id(version));
    return false;
}
[[nodiscard]] std::expected<int8_t, ble::NimbleErrorCode> rssi_value(uint16_t connectionHandle)
{
    std::expected<int8_t, ble::NimbleErrorCode> expected{};
    auto result = ble::NimbleErrorCode{ ble_gap_conn_rssi(connectionHandle, &(*expected)) };
    if (result != ble::NimbleErrorCode::success)
    {
        LOG_ERROR_FMT("Unable to retrieve rssi value. Reason: {}", ble::nimble_error_to_string(result));
        return std::unexpected(result);
    }

    return expected;
}
[[nodiscard]] uint16_t handle_of(uint16_t characteristicID)
{
    std::expected<uint16_t, ble::NimbleErrorCode> expected = ble::chr_attri_handle(ble::ID_SERVICE_WHEREAMI, characteristicID);
    ASSERT_FMT(expected,
               "Retreiving the characteristic handle must succeed for WhereAmI. Failed with {}",
               ble::nimble_error_to_string(expected.error()));

    return *expected;
}
[[nodiscard]] std::span<uint8_t> view(std::span<uint8_t> packet, uint8_t offset, uint8_t size)
{
    ASSERT(packet.size() >= static_cast<std::size_t>(offset + size), "View is outside the packets buffer!");
    return std::span<uint8_t>{ std::begin(packet) + offset, size };
}
[[nodiscard]] std::span<uint8_t> view_random_block(std::span<uint8_t> packet)
{
    static constexpr ble::DemandRSSIHeader HEADER{};
    uint8_t offset = packet[HEADER.randomDataOffset];
    uint8_t size = packet[HEADER.randomDataSize];
    return view(packet, offset, size);
}
[[nodiscard]] std::span<uint8_t> view_signature(std::span<uint8_t> packet)
{
    static constexpr ble::DemandRSSIHeader HEADER{};
    uint8_t offset = packet[HEADER.signatureOffset];
    uint8_t size = packet[HEADER.signatureSize];
    return view(packet, offset, size);
}
[[nodiscard]] std::span<uint8_t> view_hash(std::span<uint8_t> packet)
{
    static constexpr ble::DemandRSSIHeader HEADER{};
    uint8_t offset = packet[HEADER.hashOffset];
    uint8_t size = packet[HEADER.hashSize];
    return view(packet, offset, size);
}
[[nodiscard]] ble::ShaHash extract_hash(std::span<uint8_t> packet, ble::ShaVersion shaVersion)
{
    return ble::make_sha_hash(shaVersion, view_hash(packet));
}
template<typename algorithm_t>
requires security::hash_algorithm<algorithm_t>
[[nodiscard]] security::CHash<algorithm_t> make_hash(std::span<uint8_t> packet)
{
    static constexpr ble::RSSINotificationHeader HEADER{};
    uint8_t offset = packet[HEADER.randomDataOffset];
    uint8_t size = packet[HEADER.randomDataSize] + packet[HEADER.rssiSize];
    std::span<uint8_t> packetsHashBlock = view(packet, offset, size);

    return security::CHash<algorithm_t>{ packetsHashBlock };
}
[[nodiscard]] std::size_t packet_size_rssi_notification(const std::vector<uint8_t>& packet)
{
    static constexpr ble::RSSINotificationHeader HEADER{};
    return (packet[HEADER.hashSize] + packet[HEADER.randomDataSize]) + (packet[HEADER.rssiSize] +
                                                  packet[HEADER.signatureSize]) + sizeof(ble::RSSINotificationHeader);
}
}    // namespace
namespace ble
{
CWhereAmI::CWhereAmI()
    : m_pPrivateKey{ load_key<security::CEccPrivateKey>(NVS_KEY_SERVER_PRIVATE) }
    , m_pClientPublicKey{ load_key<security::CEccPublicKey>(NVS_KEY_CLIENT_PUBLIC) }
    , m_Service{}
    , m_Characteristics{}
    , m_RandomDataBlock{}
    , m_NotifyHandle{ std::nullopt }
    , m_Rssi{}
{}
CWhereAmI::CWhereAmI(const CWhereAmI& other)
    : m_pPrivateKey{ nullptr }
    , m_pClientPublicKey{ nullptr }
    , m_Service{}
    , m_Characteristics{}
    , m_RandomDataBlock{}
    , m_NotifyHandle{ std::nullopt }
    , m_Rssi{}
{
    copy(other);
}
CWhereAmI& CWhereAmI::operator=(const CWhereAmI& other)
{
    if (this != &other)
    {
        copy(other);
    }

    return *this;
}
void CWhereAmI::copy(const CWhereAmI& other)
{
    m_pPrivateKey = load_key<security::CEccPrivateKey>(NVS_KEY_SERVER_PRIVATE);
    m_pClientPublicKey = load_key<security::CEccPublicKey>(NVS_KEY_CLIENT_PUBLIC);
    m_Service = other.m_Service;
    m_Characteristics = other.m_Characteristics;
    m_RandomDataBlock = other.m_RandomDataBlock;
    m_NotifyHandle = other.m_NotifyHandle;
    m_Rssi = other.m_Rssi;
}
void CWhereAmI::register_with_nimble(const std::shared_ptr<Profile>& pProfile)
{
    m_Characteristics = make_characteristics(pProfile);
    m_Service = CGattService{ ID_SERVICE_WHEREAMI, m_Characteristics };
}
ble_gatt_svc_def CWhereAmI::as_nimble_service() const
{
    return static_cast<ble_gatt_svc_def>(m_Service);
}
bool CWhereAmI::valid_signature(ShaHash& hash, std::span<uint8_t> signature)
{
    bool verified{};
    std::visit([this, signature, &hash, &verified](auto&& h) mutable { verified = this->m_pClientPublicKey->verify_hash(signature, h); },
               hash);

    return verified;
}
std::vector<CCharacteristic> CWhereAmI::make_characteristics(const std::shared_ptr<Profile>& pProfile)
{
    std::vector<CCharacteristic> chars{};
    chars.emplace_back(make_characteristic_demand_rssi(pProfile));
    chars.emplace_back(make_characteristic_rssi_notification(pProfile));

    return chars;
}
auto CWhereAmI::make_callback_demand_rssi(const std::shared_ptr<Profile>& pProfile)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
    return
        [wpProfile =
             std::weak_ptr<Profile>{ pProfile }](uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt* pContext) -> int
    {
        std::shared_ptr<Profile> pProfile = wpProfile.lock();
        if (pProfile)
        {
            CWhereAmI* pSelf = std::get_if<CWhereAmI>(pProfile.get());
            if (pSelf != nullptr)
            {
                auto operation = CharacteristicAccess{ pContext->op };
                UNHANDLED_CASE_PROTECTION_ON
                switch (operation)
                {
                case CharacteristicAccess::write:
                {
                    NimbleErrorCode code = valid_context_buffer(pContext);
                    if (code != NimbleErrorCode::success)
                    {
                        return std::to_underlying(code);
                    }


                    LOG_INFO("WHERE_AM_I WRITE EVENT!");
                    if (pSelf->m_NotifyHandle == std::nullopt)
                    {
                        pSelf->m_NotifyHandle.emplace(handle_of(ID_CHARACTERISTIC_WHEREAMI_RSSI_NOTIFICATION));
                    }

                    std::span<uint8_t> packet{ pContext->om->om_data, std::size_t{ pContext->om->om_len } };
                    static constexpr DemandRSSIHeader HEADER{};
#ifndef NDEBUG
                    log_demand_rssi_header(packet);
#endif
                    auto shaVersion = ble::ShaVersion{ packet[HEADER.shaVersion] };
                    if (!valid_sha_version_id(shaVersion))
                    {
                        // or invalidArguments?
                        return std::to_underlying(NimbleErrorCode::unexpectedCallbackBehavior);
                    }
                    ShaHash hash = extract_hash(packet, shaVersion);
                    std::span<uint8_t> signature = view_signature(packet);

                    if (pSelf->valid_signature(hash, signature))
                    {
                        std::expected<int8_t, NimbleErrorCode> expectedRSSI = rssi_value(connectionHandle);
                        if (expectedRSSI)
                        {
                            pSelf->m_Rssi = *expectedRSSI;
                            std::span<uint8_t> randomData = view_random_block(packet);
                            pSelf->m_RandomDataBlock = std::vector<uint8_t>(std::begin(randomData), std::end(randomData));

                            [[maybe_unused]] int result = ble_gatts_notify_custom(connectionHandle, pSelf->m_NotifyHandle.value(), nullptr);
                            return std::to_underlying(ble::NimbleErrorCode::success);
                        }

                        LOG_ERROR_FMT("WhereAmI - Demand RSSI failed to retrieve RSSI value from nimble! Reason: {}",
                                      nimble_error_to_string(expectedRSSI.error()));
                        return std::to_underlying(expectedRSSI.error());
                    }
                    else
                    {
                        LOG_ERROR("Failed to verify signature recieved in Demand RSSI packet!");
                        return std::to_underlying(ble::NimbleErrorCode::invalidPeerCommand);    // maybe this code works?
                    }
                }
                case CharacteristicAccess::read:
                {
                    ASSERT_FMT(false,
                               "Write only Characteristic \"DemandRSSI\" recieved a Read operation from connection handle: \"{}\"",
                               connectionHandle);
                }
                    UNHANDLED_CASE_PROTECTION_OFF
                }
            }
            else
            {
                LOG_WARN("Characteristic callback for \"DemandRSSI\" failed to retrieve pointer to self from shared_ptr to Profile.");
            }
        }
        else
        {
            LOG_WARN(
                "Characteristic callback for \"DemandRSSI\" failed to take ownership of shared pointer to profile! It has been deleted.");
        }

        return std::to_underlying(NimbleErrorCode::unexpectedCallbackBehavior);
    };
#pragma GCC diagnostic pop
}
CCharacteristic CWhereAmI::make_characteristic_demand_rssi(const std::shared_ptr<Profile>& pProfile)
{
    return make_characteristic(ID_CHARACTERISTIC_WHEREAMI_DEMAND_RSSI, make_callback_demand_rssi(pProfile), CharsPropertyFlag::write);
}
auto CWhereAmI::make_callback_rssi_notification(const std::shared_ptr<Profile>& pProfile)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
    return [wpProfile =
                std::weak_ptr<Profile>{ pProfile }](uint16_t connectionHandle,
                                                    uint16_t attributeHandle,
                                                    ble_gatt_access_ctxt* pContext) -> int    // cppcheck-suppress constParameterPointer
    {
        std::shared_ptr<Profile> pProfile = wpProfile.lock();
        if (pProfile)
        {
            const CWhereAmI* pSelf = std::get_if<CWhereAmI>(pProfile.get());
            if (pSelf != nullptr)
            {
                auto operation = CharacteristicAccess{ pContext->op };
                switch (operation)
                {
                case CharacteristicAccess::read:
                {
                    LOG_INFO("WHERE_AM_I NOTIFY EVENT!");

                    std::vector<uint8_t> packet{};
                    packet.resize(256);

                    packet = pSelf->write_random_data_block(packet);
                    packet = pSelf->write_rssi_value(packet);

                    security::CHash<security::Sha2_256> hash = make_hash<security::Sha2_256>(packet);
                    std::vector<uint8_t> signature = pSelf->m_pPrivateKey->sign_hash(get_rng(), hash);

                    packet = pSelf->write_hash(packet, hash, ShaVersion::Sha2_256);
                    packet = pSelf->write_signature(packet, signature);

                    // Packet was allocated as a too large buffer - now with the data written it should be resized to the correct byte size
                    packet.resize(packet_size_rssi_notification(packet));
#ifndef NDEBUG
                    log_rssi_notication_header(packet);
#endif

                    NimbleErrorCode code = append_read_data(pContext->om, packet);
                    if (code != NimbleErrorCode::success)
                    {
                        LOG_ERROR_FMT("Characteristic callback for \"RSSI Notification\" failed to append its data to the client: \"{}\"",
                                      nimble_error_to_string(code));
                    }

                    return std::to_underlying(code);
                }
                }
            }
            else
            {
                LOG_WARN(
                    "Characteristic callback for \"RSSI Notification\" failed to retrieve pointer to self from shared_ptr to Profile.");
            }
        }
        else
        {
            LOG_WARN("Characteristic callback for \"RSSI Notification\" failed to take ownership of shared pointer to profile! It has been "
                     "deleted.");
        }

        return static_cast<int32_t>(NimbleErrorCode::unexpectedCallbackBehavior);
    };
#pragma GCC diagnostic pop
}
CCharacteristic CWhereAmI::make_characteristic_rssi_notification(const std::shared_ptr<Profile>& pProfile)
{
    return make_characteristic(ID_CHARACTERISTIC_WHEREAMI_RSSI_NOTIFICATION,
                               make_callback_rssi_notification(pProfile),
                               CharsPropertyFlag::notify);
}
std::vector<uint8_t> CWhereAmI::write_random_data_block(std::vector<uint8_t>& packet) const
{
    static constexpr RSSINotificationHeader HEADER{};

    uint8_t offset = sizeof(RSSINotificationHeader);
    uint8_t size = m_RandomDataBlock.size();

    packet[HEADER.randomDataOffset] = offset;
    packet[HEADER.randomDataSize] = size;

    std::span<uint8_t> packetsRandomBlock = view(packet, offset, size);
    std::size_t smallestSize = packetsRandomBlock.size() <= m_RandomDataBlock.size() ? packetsRandomBlock.size() : m_RandomDataBlock.size();
    std::memcpy(packetsRandomBlock.data(), m_RandomDataBlock.data(), smallestSize);

    return packet;
}
std::vector<uint8_t> CWhereAmI::write_rssi_value(std::vector<uint8_t>& packet) const
{
    static constexpr RSSINotificationHeader HEADER{};

    uint8_t offset = packet[HEADER.randomDataOffset] + packet[HEADER.randomDataSize];
    uint8_t size = sizeof(decltype(m_Rssi));

    packet[HEADER.rssiOffset] = offset;
    packet[HEADER.rssiSize] = size;

    packet[offset] = m_Rssi;

    return packet;
}
std::vector<uint8_t> CWhereAmI::write_signature(std::vector<uint8_t>& packet, const std::vector<uint8_t>& signature) const
{
    static constexpr RSSINotificationHeader HEADER{};

    uint8_t offset = packet[HEADER.hashOffset] + packet[HEADER.hashSize];
    uint8_t size = signature.size();

    packet[HEADER.signatureOffset] = offset;
    packet[HEADER.signatureSize] = size;

    std::span<uint8_t> signatureBlock = view(packet, offset, size);
    std::size_t smallestSize = signatureBlock.size() <= signature.size() ? signatureBlock.size() : signature.size();
    std::memcpy(signatureBlock.data(), signature.data(), smallestSize);

    return packet;
}
}    // namespace ble
