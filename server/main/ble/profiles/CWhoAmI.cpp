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
    if(this != &other)
    {
        copy(other);
    }
    return *this;
}
void CWhoAmI::copy(const CWhoAmI& other)
{
    m_pPrivateKey = load_key<security::CEccPrivateKey>(NVS_KEY_SERVER_PRIVATE);
    m_SignedMacData = other.m_SignedMacData;
    m_Characteristics = std::vector<CCharacteristic>{};
    m_Service = CGattService{};
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
    security::CHash<security::Sha2_256> hash{ std::move(addressResult.value())};
    std::vector<security::byte> signature = m_pPrivateKey->sign_hash(rng, hash);

    size_t packetSize = sizeof(ServerAuthHeader) + hash.size() + signature.size();
    std::vector<security::byte> packetData{ common::assert_down_cast<uint8_t>(static_cast<uint8_t>(HashType::Sha2_256)),
                                            common::assert_down_cast<uint8_t>(sizeof(ServerAuthHeader)),
                                            common::assert_down_cast<uint8_t>(hash.size()),
                                            common::assert_down_cast<uint8_t>(sizeof(ServerAuthHeader) + hash.size()),
                                            common::assert_down_cast<uint8_t>(signature.size()) };
    packetData.resize(packetSize);
    std::memcpy(packetData.data() + sizeof(ServerAuthHeader), hash.data(), hash.size());
    std::memcpy((packetData.data() + hash.size() + sizeof(ServerAuthHeader)), signature.data(), signature.size() * sizeof(security::byte));
    
    m_SignedMacData = std::move(packetData);
    if (m_SignedMacData.empty())
    {
    	LOG_FATAL("Failed to sign server MAC address!");
    }
}
ble_gatt_svc_def CWhoAmI::as_nimble_service() const
{
    return static_cast<ble_gatt_svc_def>(m_Service);
}
std::vector<CCharacteristic> CWhoAmI::make_characteristics(const std::shared_ptr<Profile>& pProfile)
{
    std::vector<CCharacteristic> chars{};
    chars.emplace_back(make_characteristic_server_auth(pProfile));
    return chars;
}
auto CWhoAmI::make_callback_server_auth(const std::shared_ptr<Profile>& pProfile)
{
    return [wpProfile = std::weak_ptr<Profile>{ pProfile }](uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt* pContext) -> int	// type deduction requires exact typematch
    {
        std::shared_ptr<Profile> pProfile = wpProfile.lock();
        if(pProfile)
        {
            CWhoAmI* pSelf = std::get_if<CWhoAmI>(pProfile.get());
            if(pSelf != nullptr)
            {
                auto operation = CharacteristicAccess{ pContext->op };
                UNHANDLED_CASE_PROTECTION_ON
                switch (operation) 
                {
                    case CharacteristicAccess::read:
                    {
                        if(pSelf->m_SignedMacData.empty())
                            pSelf->sign_server_mac_address();

                        NimbleErrorCode code = append_read_data(pContext->om, pSelf->m_SignedMacData);
                        if(code != NimbleErrorCode::success)
                        {
                            LOG_ERROR_FMT("Characteristic callback for Server Auth failed to append its data to the client: \"{}\"", 
                                            nimble_error_to_string(code));
                        }

                        return static_cast<int32_t>(code);
                    }
                    case CharacteristicAccess::write:
                    {
                    	LOG_ERROR_FMT("Read only Characteristic \"Server Auth\" recieved a Write operation from connection handle: \"{}\"",
                    					connectionHandle);
                    }
                }
                UNHANDLED_CASE_PROTECTION_OFF
            }
            else
            {
            	LOG_WARN("Characteristic callback for \"Server Auth\" failed to retrieve pointer to self from shared_ptr to Profile.");
            }
        }
        else
        {
        	LOG_WARN("Characteristic callback for \"Server Auth\" failed to take ownership of shared pointer to profile! It has been deleted.");
        }

        return static_cast<int32_t>(NimbleErrorCode::unexpectedCallbackBehavior);
    };
}
CCharacteristic CWhoAmI::make_characteristic_server_auth(const std::shared_ptr<Profile>& pProfile)
{
	return make_characteristic(ID_CHARACTERISTIC_SERVER_AUTH, make_callback_server_auth(pProfile), CharsPropertyFlag::read);
}
}	// namespace ble