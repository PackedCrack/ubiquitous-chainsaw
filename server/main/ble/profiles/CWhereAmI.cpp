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
[[nodiscard]] std::span<uint8_t> view(const std::span<uint8_t> packet, uint8_t offset, uint8_t size)
{
    ASSERT(packet.size() >= static_cast<std::size_t>(offset + size), "View is outside the packets buffer!");
    return std::span<uint8_t>{ std::begin(packet) + offset, size };
}
[[nodiscard]] std::span<uint8_t> view_random_block(const std::span<uint8_t> packet)
{
    static constexpr ble::DemandRSSIHeader HEADER{};
    uint8_t offset = packet[HEADER.randomDataOffset];
    uint8_t size = packet[HEADER.randomDataSize];
    return view(packet, offset, size);
}
[[nodiscard]] std::span<uint8_t> view_signature(const std::span<uint8_t> packet)
{
    static constexpr ble::DemandRSSIHeader HEADER{};
    uint8_t offset = packet[HEADER.signatureOffset];
    uint8_t size = packet[HEADER.signatureSize];
    return view(packet, offset, size);
}
[[nodiscard]] std::span<uint8_t> view_hash(const std::span<uint8_t> packet)
{
    static constexpr ble::DemandRSSIHeader HEADER{};
    uint8_t offset = packet[HEADER.hashOffset];
    uint8_t size = packet[HEADER.hashSize];
    return view(packet, offset, size);
}
[[nodiscard]] ble::ShaHash make_hash(const std::span<uint8_t> packet, ble::ShaVersion shaVersion)
{
    return ble::make_sha_hash(shaVersion, view_hash(packet));
}
}    // namespace
namespace ble
{
CWhereAmI::CWhereAmI()
    : m_Rssi{}
    , m_NotifyHandle{ std::nullopt }
    , m_pPrivateKey{ load_key<security::CEccPrivateKey>(NVS_KEY_SERVER_PRIVATE) }
    , m_pClientPublicKey{ load_key<security::CEccPublicKey>(NVS_KEY_CLIENT_PUBLIC) }
    , m_Characteristics{}
    , m_Service{}
{}
CWhereAmI::CWhereAmI(const CWhereAmI& other)
    : m_Rssi{}
    , m_NotifyHandle{ std::nullopt }
    , m_pPrivateKey{ nullptr }
    , m_pClientPublicKey{ nullptr }
    , m_Characteristics{}
    , m_Service{}
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
    m_Rssi = other.m_Rssi;
    m_NotifyHandle = other.m_NotifyHandle;
    m_pPrivateKey = load_key<security::CEccPrivateKey>(NVS_KEY_SERVER_PRIVATE);
    m_pClientPublicKey = load_key<security::CEccPublicKey>(NVS_KEY_CLIENT_PUBLIC);
    m_Characteristics = other.m_Characteristics;
    m_Service = other.m_Service;
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
    std::visit(
        [this, signature, &hash, &verified](auto&& h) mutable
        {
            verified = this->m_pClientPublicKey->verify_hash(signature, h);

            if (verified)
            {
                LOG_INFO("AAAAAAAAAAAAAAAAA VERIFIED AAAAAAAAAAAAAAAAAAAAA");
            }
        },
        hash);

    return verified;
}
std::vector<CCharacteristic> CWhereAmI::make_characteristics(const std::shared_ptr<Profile>& pProfile)
{
    std::vector<CCharacteristic> chars{};
    chars.emplace_back(make_characteristic_demand_rssi(pProfile));
    chars.emplace_back(make_characteristic_send_rssi(pProfile));

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
                        pSelf->m_NotifyHandle.emplace(handle_of(ID_CHARACTERISTIC_WHEREAMI_SEND_RSSI));
                    }

                    std::span<uint8_t> packet{ pContext->om->om_data, std::span<uint8_t>::size_type{ pContext->om->om_len } };
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
                    ShaHash hash = make_hash(packet, shaVersion);
                    std::span<uint8_t> signature = view_signature(packet);
                    std::span<uint8_t> randomData = view_random_block(packet);
                    if (pSelf->valid_signature(hash, signature))
                    {
                        std::expected<int8_t, NimbleErrorCode> expected = rssi_value(connectionHandle);
                        if (!expected)
                        {
                            return std::to_underlying(expected.error());
                        }

                        pSelf->m_Rssi = *expected;
                        std::printf("Rssi: %i\n", pSelf->m_Rssi);

                        int result = ble_gatts_notify_custom(connectionHandle, pSelf->m_NotifyHandle.value(), nullptr);
                        LOG_INFO_FMT("notify result: {}", result);

                        return std::to_underlying(ble::NimbleErrorCode::success);
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
                LOG_WARN("Characteristic callback for \"Server Auth\" failed to retrieve pointer to self from shared_ptr to Profile.");
            }
        }
        else
        {
            LOG_WARN(
                "Characteristic callback for \"Server Auth\" failed to take ownership of shared pointer to profile! It has been deleted.");
        }

        return std::to_underlying(NimbleErrorCode::unexpectedCallbackBehavior);
    };
#pragma GCC diagnostic pop
}
CCharacteristic CWhereAmI::make_characteristic_demand_rssi(const std::shared_ptr<Profile>& pProfile)
{
    return make_characteristic(ID_CHARACTERISTIC_WHEREAMI_DEMAND_RSSI, make_callback_demand_rssi(pProfile), CharsPropertyFlag::write);
}
auto CWhereAmI::make_callback_send_rssi(const std::shared_ptr<Profile>& pProfile)
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


                    // NimbleErrorCode code = append_read_data(pContext->om, rssiValue);
                    // if(code != NimbleErrorCode::success)
                    //{
                    //     LOG_ERROR_FMT("Characteristic callback for Server Auth failed to append its data to the client: \"{}\"",
                    //                     nimble_error_to_string(code));
                    // }


                    return static_cast<int32_t>(ble::NimbleErrorCode::success);
                }
                }
            }
            else
            {
                LOG_WARN("Characteristic callback for \"Server Auth\" failed to retrieve pointer to self from shared_ptr to Profile.");
            }
        }
        else
        {
            LOG_WARN(
                "Characteristic callback for \"Server Auth\" failed to take ownership of shared pointer to profile! It has been deleted.");
        }

        return static_cast<int32_t>(NimbleErrorCode::unexpectedCallbackBehavior);
    };
#pragma GCC diagnostic pop
}
CCharacteristic CWhereAmI::make_characteristic_send_rssi(const std::shared_ptr<Profile>& pProfile)
{
    return make_characteristic(ID_CHARACTERISTIC_WHEREAMI_SEND_RSSI, make_callback_send_rssi(pProfile), CharsPropertyFlag::notify);
}
}    // namespace ble
