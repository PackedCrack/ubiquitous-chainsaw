//
// Created by hackerman on 1/20/24.
//
#include "CScanner.hpp"
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>


static constexpr char NULL_TERMINATOR = 0;
static constexpr int32_t FAILURE = -1;
static constexpr int32_t SOCKET_NOT_OPENED = -1;

namespace
{
    enum class InternalError
    {
        AddressToStr,
        NameLookUp
    };
    [[nodiscard]] const char* str_error()
    {
        std::array<char, 256> buffer{ NULL_TERMINATOR };
        return strerror_r(errno, buffer.data(), buffer.size());
    }
    template<typename str_t>
    [[nodiscard]] std::unexpected<bluetooth::CScanner::Error> error(bluetooth::CScanner::ErrorCode, str_t&& str)
    {
        using namespace bluetooth;
        return std::unexpected<CScanner::Error>{
                CScanner::Error{
                        .code = CScanner::ErrorCode::constructError,
                        .msg = std::forward<str_t>(str)
                }
        };
    }
    [[nodiscard]] bool device_available()
    {
        int32_t SCANNING = 1;
        int32_t NOT_SCANNING = 0;
        static std::atomic<int32_t> hardwareState{ NOT_SCANNING };
        
        return hardwareState.compare_exchange_weak(NOT_SCANNING, SCANNING, std::memory_order_release, std::memory_order_relaxed);
    }
    [[nodiscard]] uint32_t as_combined_value(const uint8_t* pDeviceClass)
    {
        uint32_t combined = static_cast<uint32_t>(*pDeviceClass << 16u);
        ++pDeviceClass;
        combined = combined | static_cast<uint32_t>(*pDeviceClass << 8u);
        ++pDeviceClass;
        combined = combined | static_cast<uint32_t>(*pDeviceClass);
        
        return combined;
    }
    [[nodiscard]] std::vector<bluetooth::ServiceClass> extract_service_classes(const uint8_t* pDeviceClass)
    {
        using namespace bluetooth;
        static constexpr uint32_t SERVICE_CLASS_BITS_USED = 0xFFE000;
        
        std::vector<ServiceClass> services{};
        uint32_t serviceFlags = as_combined_value(pDeviceClass) & SERVICE_CLASS_BITS_USED;
        if(serviceFlags & std::to_underlying(ServiceClass::LimitedDiscoverableMode))
            services.emplace_back(ServiceClass::LimitedDiscoverableMode);
        if(serviceFlags & std::to_underlying(ServiceClass::LEAudio))
            services.emplace_back(ServiceClass::LEAudio);
        if(serviceFlags & std::to_underlying(ServiceClass::Reserved))
            services.emplace_back(ServiceClass::Reserved);
        if(serviceFlags & std::to_underlying(ServiceClass::Positioning))
            services.emplace_back(ServiceClass::Positioning);
        if(serviceFlags & std::to_underlying(ServiceClass::Networking))
            services.emplace_back(ServiceClass::Networking);
        if(serviceFlags & std::to_underlying(ServiceClass::Rendering))
            services.emplace_back(ServiceClass::Rendering);
        if(serviceFlags & std::to_underlying(ServiceClass::Capture))
            services.emplace_back(ServiceClass::Capture);
        if(serviceFlags & std::to_underlying(ServiceClass::ObjectTransfer))
            services.emplace_back(ServiceClass::ObjectTransfer);
        if(serviceFlags & std::to_underlying(ServiceClass::Telephony))
            services.emplace_back(ServiceClass::Telephony);
        if(serviceFlags & std::to_underlying(ServiceClass::Information))
            services.emplace_back(ServiceClass::Information);
        
        return services;
    }
    [[nodiscard]] bluetooth::MajorDeviceClass extract_major_device_class(const uint8_t* pDeviceClass)
    {
        static constexpr uint32_t MAJOR_CLASS_BITS_USED = 0x1F00;
        
        return bluetooth::MajorDeviceClass{ as_combined_value(pDeviceClass) & MAJOR_CLASS_BITS_USED };
    }
    [[nodiscard]] std::expected<std::string, InternalError> device_name_lookup(int32_t socket, const bdaddr_t& addr)
    {
        ASSERT(socket >= 0, "Invalid socket");
        static constexpr int32_t TIMEOUT = 0;
        
        std::array<char, 256u> buffer{ NULL_TERMINATOR };
        int32_t result = hci_read_remote_name(socket, &addr, buffer.size(), buffer.data(), TIMEOUT);
        if(result == FAILURE)
            return std::unexpected{ InternalError::NameLookUp };
        
        return std::string{ buffer.data() };
    }
    [[nodiscard]] std::expected<std::string, InternalError> log_name_lookup_err(InternalError err)
    {
        LOG_WARN_FMT("Failed to look up device name. InternalError code: {} errno: {}",
                     std::to_underlying(err),
                     str_error());
        
        return std::string{ "Lookup Failure" };
    }
    [[nodiscard]] std::expected<std::string, InternalError> device_addr_to_str(const bdaddr_t& addr)
    {
        std::array<char, 32> buffer{ NULL_TERMINATOR };
        int32_t result = ba2str(&addr, buffer.data());
        if(result == FAILURE)
            return std::unexpected{ InternalError::AddressToStr };
        
        return std::string{ buffer.data() };
    }
    [[nodiscard]] std::expected<std::string, InternalError> log_addr_conversion_err(InternalError err)
    {
        LOG_ERROR_FMT("Failed to convert Bluetooth Device Address to string InternalError code: {} errno: {}",
                      std::to_underlying(err),
                      str_error());
        
        return std::string{ "Conversion Failure" };
    }
}   // namespace

namespace bluetooth
{
std::expected<CScanner, CScanner::Error> CScanner::make_scanner()
{
    try
    {
        return CScanner{};
    }
    catch(const std::runtime_error& err)
    {
        return error(ErrorCode::constructError, err.what());
    }
}
CScanner::CScanner()
    : m_DeviceID{ hci_get_route(nullptr) }
    , m_Socket{ SOCKET_NOT_OPENED }
{
    if(m_DeviceID == FAILURE)
        throw std::runtime_error{ "Failed to find a Bluetooth Adapter device ID" };
    
    m_Socket = hci_open_dev(m_DeviceID);
    if(m_Socket == FAILURE)
    {
        std::array<char, 256> buffer{ NULL_TERMINATOR };
        throw std::runtime_error{ std::format("Failed to open a socket to the local Bluetooth Adapter: {}", str_error()) };
    }
}

CScanner::~CScanner()
{
    if(m_Socket == SOCKET_NOT_OPENED)
        return;
    
    close(m_Socket);
}

CScanner::CScanner(CScanner&& other) noexcept
    : m_DeviceID{ other.m_DeviceID }
    , m_Socket{ other.m_Socket }
{
    other.m_DeviceID = -1;
    other.m_Socket = SOCKET_NOT_OPENED;
}

CScanner& CScanner::operator=(CScanner&& other) noexcept
{
    m_DeviceID = other.m_DeviceID;
    other.m_DeviceID = -1;
    
    m_Socket = other.m_Socket;
    other.m_Socket = SOCKET_NOT_OPENED;
    
    return *this;
}

std::expected<std::vector<Inquiry>, CScanner::Error> CScanner::new_scan() const
{
    ASSERT(m_DeviceID >= 0, "Invalid device id");
    
    if(!device_available())
        return error(ErrorCode::scanInProgress, "A scan is already in progress.");
    
    /*
     * The inquiry lasts for at most 1.28 * len seconds, and at most max_rsp devices will be returned in the output parameter ii,
     * which must be large enough to accommodate max_rsp results. We suggest using a max_rsp of 255 for a standard 10.24 second inquiry.
     * https://people.csail.mit.edu/albert/bluez-intro/c404.html
     */
    static constexpr int32_t LEN = 8;
    static constexpr int32_t MAX_RSP = 255;
    
    std::vector<inquiry_info> responses{ MAX_RSP };
    ASSERT(responses.size() == MAX_RSP, "Responses buffer not large enough");
    
    inquiry_info* pBuffer = responses.data();
    int32_t numResponses = hci_inquiry(m_DeviceID, LEN, MAX_RSP, nullptr, &pBuffer, IREQ_CACHE_FLUSH);
    if(numResponses == FAILURE)
        return error(ErrorCode::responseFailure, std::format("Scan failed with error: {}", str_error()));
    responses.resize(static_cast<size_t>(numResponses));
    
    LOG_INFO_FMT("DeviceID: {}. Socket: {}, Num responses: {}", m_DeviceID, m_Socket, numResponses);
    std::vector<Inquiry> inquiries{};
    for(auto& inquiry : responses)
    {
        inquiries.emplace_back(Inquiry{
                .name = device_name_lookup(m_Socket, inquiry.bdaddr).or_else(log_name_lookup_err).value(),
                .addr = device_addr_to_str(inquiry.bdaddr).or_else(log_addr_conversion_err).value(),
                .services = extract_service_classes(&inquiry.dev_class[0]),
                .majorClass = extract_major_device_class(&inquiry.dev_class[0])
        });
    }
    
    return inquiries;
}
}   // namespace