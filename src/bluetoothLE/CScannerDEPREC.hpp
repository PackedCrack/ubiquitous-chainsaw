////
//// Created by hackerman on 1/20/24.
////
//
//#pragma once
//#include "../defines.hpp"
//
//
//namespace bluetooth
//{
//enum class ServiceClass : uint32_t
//{
//    LimitedDiscoverableMode = 1u << 13u,
//    LEAudio = 1u << 14u,
//    Reserved = 1u << 15u,
//    Positioning = 1u << 16u,
//    Networking = 1u << 17u,
//    Rendering = 1u << 18u,
//    Capture = 1u << 19u,
//    ObjectTransfer = 1u << 20u,
//    Audio = 1u << 21u,
//    Telephony = 1u << 22u,
//    Information = 1u << 23u
//};
//[[nodiscard]] constexpr std::string_view service_class_to_str(ServiceClass service)
//{
//    switch(std::to_underlying(service))
//    {
//        case std::to_underlying(ServiceClass::LimitedDiscoverableMode):
//        { static constexpr std::string_view asStr = "Limited Discoverable Mode"; return asStr; }
//        case std::to_underlying(ServiceClass::LEAudio):
//        { static constexpr std::string_view asStr = "LE Audio"; return asStr; }
//        case std::to_underlying(ServiceClass::Reserved):
//        { static constexpr std::string_view asStr = "Reserved"; return asStr; }
//        case std::to_underlying(ServiceClass::Positioning):
//        { static constexpr std::string_view asStr = "Positioning"; return asStr; }
//        case std::to_underlying(ServiceClass::Networking):
//        { static constexpr std::string_view asStr = "Networking"; return asStr; }
//        case std::to_underlying(ServiceClass::Rendering):
//        { static constexpr std::string_view asStr = "Rendering"; return asStr; }
//        case std::to_underlying(ServiceClass::Capture):
//        { static constexpr std::string_view asStr = "Capture"; return asStr; }
//        case std::to_underlying(ServiceClass::ObjectTransfer):
//        { static constexpr std::string_view asStr = "Object Transfer"; return asStr; }
//        case std::to_underlying(ServiceClass::Audio):
//        { static constexpr std::string_view asStr = "Audio"; return asStr; }
//        case std::to_underlying(ServiceClass::Telephony):
//        { static constexpr std::string_view asStr = "Telephony"; return asStr; }
//        case std::to_underlying(ServiceClass::Information):
//        { static constexpr std::string_view asStr = "Information"; return asStr; }
//    }
//    
//    ASSERT(false, "Missing Service Class Entry");
//    std::unreachable();
//}
//enum class MajorDeviceClass : uint32_t
//{
//    Miscellaneous = 0u,
//    Computer = 1u << 8u,
//    Phone = 1u << 9u,
//    LANOrNetworkAccessPoint = 3u << 8,
//    AudioOrVideo = 1u << 10u,
//    Peripheral = 5u << 8u,
//    Imaging = 3u << 9u,
//    Wearable = 7u << 8u,
//    Toy = 1u << 11u,
//    Health = 9u << 8u,
//    Uncategorized = 31u << 8u
//};
//[[nodiscard]] constexpr std::string_view major_device_class_to_str(MajorDeviceClass service)
//{
//    switch(std::to_underlying(service))
//    {
//        case std::to_underlying(MajorDeviceClass::Miscellaneous):
//        { static constexpr std::string_view asStr = "Miscellaneous"; return asStr; }
//        case std::to_underlying(MajorDeviceClass::Computer):
//        { static constexpr std::string_view asStr = "Computer"; return asStr; }
//        case std::to_underlying(MajorDeviceClass::Phone):
//        { static constexpr std::string_view asStr = "Phone"; return asStr; }
//        case std::to_underlying(MajorDeviceClass::LANOrNetworkAccessPoint):
//        { static constexpr std::string_view asStr = "LAN/Network Access Point"; return asStr; }
//        case std::to_underlying(MajorDeviceClass::AudioOrVideo):
//        { static constexpr std::string_view asStr = "Audio/Video"; return asStr; }
//        case std::to_underlying(MajorDeviceClass::Peripheral):
//        { static constexpr std::string_view asStr = "Peripheral"; return asStr; }
//        case std::to_underlying(MajorDeviceClass::Imaging):
//        { static constexpr std::string_view asStr = "Imaging"; return asStr; }
//        case std::to_underlying(MajorDeviceClass::Wearable):
//        { static constexpr std::string_view asStr = "Wearable"; return asStr; }
//        case std::to_underlying(MajorDeviceClass::Toy):
//        { static constexpr std::string_view asStr = "Toy"; return asStr; }
//        case std::to_underlying(MajorDeviceClass::Health):
//        { static constexpr std::string_view asStr = "Health"; return asStr; }
//        case std::to_underlying(MajorDeviceClass::Uncategorized):
//        { static constexpr std::string_view asStr = "Uncategorized"; return asStr; }
//    }
//    
//    ASSERT(false, "Missing Major Device Class Entry");
//    std::unreachable();
//}
//// Todo: implement all minor classes: https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Assigned_Numbers/out/en/Assigned_Numbers.pdf?v=1705773920186
//enum class MinorDeviceClassComputer : uint32_t
//{
//    Uncategorized = 0u,
//    DesktopWorkstation = 1u << 2u,
//    ServerClassComputer = 1u << 3u,
//    Laptop = 3u << 2u,
//    HandheldPCOrPDA = 1u << 4u,
//    PalmSizePCOrPDA = 5u << 2u,
//    WearableComputer = 3u << 3u,
//    Tablet = 7u << 2u
//};
//struct Inquiry
//{
//    std::string name;
//    std::string addr;
//    std::vector<ServiceClass> services;
//    MajorDeviceClass majorClass;
//};
//
//class CScanner
//{
//public:
//    enum class ErrorCode
//    {
//        constructError,
//        scanInProgress,
//        responseFailure,
//        count
//    };
//    struct Error
//    {
//        ErrorCode code;
//        std::string msg;
//    };
//private:
//    CScanner();
//public:
//    [[nodiscard]] static std::expected<CScanner, Error> make_scanner();
//    ~CScanner();
//    CScanner(const CScanner& other) = delete;
//    CScanner(CScanner&& other) noexcept;
//    CScanner& operator=(const CScanner& other) = delete;
//    CScanner& operator=(CScanner&& other) noexcept;
//public:
//    [[nodiscard]] std::expected<std::vector<Inquiry>, CScanner::Error>  new_scan() const;
//private:
//    int32_t m_DeviceID;
//    // Socket to local Bluetooth adapter, NOT connection to a remote Bluetooth device.
//    int32_t m_Socket;
//};
//}   // namespace bluetooth