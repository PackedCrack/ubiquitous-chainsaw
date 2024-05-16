// Wolfcrypt must be included BEFORE windows.h
#include "system/System.hpp"
#include "system/TrayIcon.hpp"

#include "security/CHash.hpp"
#include "security/sha.hpp"
#include "security/CWolfCrypt.hpp"
#include "security/CRandom.hpp"
#include "security/ecc_key.hpp"

#include "client_defines.hpp"
#include "gfx/CRenderer.hpp"
#include "gui/CGui.hpp"
#include "common/CStopWatch.hpp"
#include "CServer.hpp"

#include <winrt/Windows.Foundation.h>
#include <winrt/windows.storage.streams.h>

#include "bluetoothLE/Scanner.hpp"
#include "bluetoothLE/Device.hpp"
#include "gui/CDeviceList.hpp"
#include "CRssiDemander.hpp"
#include "client_common.hpp"

#include "common/CCoroutineManager.hpp"
// clang-format off


// clang-format on
namespace
{
void validate_app_directory()
{
    auto validate_app_directory = [](const std::filesystem::path& appDirectory)
    {
        if (!std::filesystem::exists(appDirectory))
        {
            std::filesystem::create_directory(appDirectory);
        }

        ASSERT_FMT(std::filesystem::is_directory(appDirectory), "Expected {} to be a directory.", appDirectory.string());
        return sys::key_directory();
    };
    auto validate_key_directory = [](const std::filesystem::path& keyLocation)
    {
        if (!std::filesystem::exists(keyLocation))
        {
            std::filesystem::create_directory(keyLocation);
        }

        ASSERT_FMT(std::filesystem::is_directory(keyLocation), "Expected {} to be a directory.", keyLocation.string());
        return std::expected<std::filesystem::path, std::string>{};
    };
    auto log_failure = [](const std::string& err)
    {
        LOG_FATAL_FMT("Could not validate Application Directory because key location could not be retrieved. Reason: {}", err);
        return std::expected<std::filesystem::path, std::string>{};
    };


    // clang-format off
    [[maybe_unused]] auto result =
        sys::application_directory()
        .and_then(validate_app_directory)
        .and_then(validate_key_directory)
        .or_else(log_failure);
    // clang-format on
}
void process_cmd_line_args(int argc, char** argv)
{
    if (argc < 1)
    {
        return;
    }

    for (int32_t i = 1; i < argc; ++i)
    {
        std::string_view arg{ argv[i] };
        if (arg == "--make-keys")
        {
            {
                auto [pubKey, privKey] = make_ecc_keys();
                save_ecc_keys(pubKey, privKey, SERVER_PUBLIC_KEY_NAME, SERVER_PRIVATE_KEY_NAME);
            }
            {
                auto [pubKey, privKey] = make_ecc_keys();
                save_ecc_keys(pubKey, privKey, CLIENT_PUBLIC_KEY_NAME, CLIENT_PRIVATE_KEY_NAME);
            }
        }
        if (arg == "--print-keys")
        {
            auto print_key = [](std::string_view keyName)
            {
                return [keyName](const std::vector<byte>& key)
                {
                    std::stringstream sstream{};
                    for (auto&& byte : key)
                    {
                        sstream << std::setw(2) << std::setfill('0') << std::hex << static_cast<int32_t>(byte);
                    }
                    LOG_INFO_FMT("{}: {}", keyName, sstream.str());
                    return std::expected<std::vector<byte>, std::string>{};
                };
            };
            auto log_failure = [](const std::string& err)
            {
                LOG_ERROR_FMT("Could not print key contents. Reason: \"{}\"", err);

                return std::expected<std::vector<byte>, std::string>{};
            };

            static constexpr std::array<std::string_view, 4u> keyNames = { CLIENT_PUBLIC_KEY_NAME,
                                                                           CLIENT_PRIVATE_KEY_NAME,
                                                                           SERVER_PUBLIC_KEY_NAME,
                                                                           SERVER_PRIVATE_KEY_NAME };
            for (auto&& keyName : keyNames)
            {
                auto load_key = make_invokable_load_file(keyName);
                // clang-format off
                [[maybe_unused]] auto result = load_key()
                        .and_then(print_key(keyName))
                        .or_else(log_failure);
                // clang-format on
            }
        }
    }
}
[[nodiscard]] std::shared_ptr<CRssiDemander> make_rssi_demander(CServer& server, gfx::CWindow& window)
{
    return std::make_shared<CRssiDemander>(server, window, std::chrono::seconds(1));
}
}    // namespace
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <ntddser.h>
#include <regstr.h>
#include <iostream>

#pragma comment(lib, "setupapi.lib")
#include "system/windows/CDeviceInfoSet.hpp"
#include "system/SerialCommunication.hpp"
namespace
{}    // namespace
int main(int argc, char** argv)
{
    std::expected<sys::CSerialCommunication, sys::ErrorSerialCom> expected = sys::open_serial_communication();


    sys::CSerialCommunication serial = sys::CSerialCommunication::make().value();


    sys::CDeviceInfoSet deviceInfoSet{ GUID_DEVINTERFACE_COMPORT };
    std::vector<std::string> infos = deviceInfoSet.enumerate_device_info(sys::CDeviceInfoSet::Property::hardwareID);
    std::vector<std::string> infos2 = deviceInfoSet.enumerate_device_info(sys::CDeviceInfoSet::Property::friendlyName);
    for (auto&& info : infos)
    {
        LOG_INFO_FMT("Info: {}", info.c_str());
    }
    for (auto&& info : infos2)
    {
        LOG_INFO_FMT("Info: {}", info.c_str());
    }


    //// https://learn.microsoft.com/en-us/windows/win32/api/setupapi/ns-setupapi-sp_devinfo_data
    //SP_DEVINFO_DATA deviceInfoData{};
    //deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    //SP_DEVICE_INTERFACE_DATA deviceInterfaceData{};
    //deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    //DWORD index = 0;
    //// https://learn.microsoft.com/en-us/windows/win32/api/setupapi/nf-setupapi-setupdienumdeviceinfo
    //while (SetupDiEnumDeviceInterfaces(deviceInfoSet.m_pDeviceInfoSet, nullptr, &GUID_DEVINTERFACE_COMPORT, index, &deviceInterfaceData) ==
    //       TRUE)
    //{
    //    DWORD requiredSize{};
    //    // https://learn.microsoft.com/en-us/windows/win32/api/setupapi/nf-setupapi-setupdigetdeviceinterfacedetaila
    //    SetupDiGetDeviceInterfaceDetail(deviceInfoSet.m_pDeviceInfoSet, &deviceInterfaceData, nullptr, 0, &requiredSize, nullptr);
    //    // returns the required buffer size at RequiredSize and fails with GetLastError returning ERROR_INSUFFICIENT_BUFFER
    //    ASSERT(GetLastError() == ERROR_INSUFFICIENT_BUFFER, "Working as intended. Good job Microsoft");


    //    auto pDetailData = static_cast<SP_DEVICE_INTERFACE_DETAIL_DATA*>(std::malloc(requiredSize));
    //    ASSERT(pDetailData, "Failed to allocate buffer for Device Details");
    //    pDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    //    WIN_CHECK(SetupDiGetDeviceInterfaceDetail(deviceInfoSet.m_pDeviceInfoSet,
    //                                              &deviceInterfaceData,
    //                                              pDetailData,
    //                                              requiredSize,
    //                                              nullptr,
    //                                              &deviceInfoData));

    //    LOG_INFO_FMT("Device Path: {}", pDetailData->DevicePath);


    //    std::free(pDetailData);

    //    std::array<BYTE, 512> buffer{};
    //    // https://learn.microsoft.com/en-us/windows/win32/api/setupapi/nf-setupapi-setupdigetdeviceregistrypropertya
    //    WIN_CHECK(SetupDiGetDeviceRegistryProperty(deviceInfoSet.m_pDeviceInfoSet,
    //                                               &deviceInfoData,
    //                                               SPDRP_FRIENDLYNAME,
    //                                               nullptr,
    //                                               buffer.data(),
    //                                               static_cast<DWORD>(buffer.size()),
    //                                               &requiredSize));
    //    ASSERT(requiredSize <= buffer.size(), "Pre allocated buffer size to small.");

    //    LOG_INFO_FMT("Device Property Output: {}", std::string{ std::begin(buffer), std::end(buffer) }.c_str());

    //    //deviceInfo.emplace_back(std::begin(buffer), std::end(buffer));

    //    ++index;
    //}
    //WIN_CHECK(GetLastError() == ERROR_NO_MORE_ITEMS);


    //HDEVINFO pDeviceInfoSet = nullptr;
    //// https://learn.microsoft.com/en-us/windows/win32/api/setupapi/nf-setupapi-setupdigetclassdevsw
    //WIN_CHECK(pDeviceInfoSet = SetupDiGetClassDevs(&GUID_DEVINTERFACE_COMPORT, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    //          pDeviceInfoSet != INVALID_HANDLE_VALUE);

    //// https://learn.microsoft.com/en-us/windows/win32/api/setupapi/ns-setupapi-sp_devinfo_data
    //SP_DEVINFO_DATA deviceInfoData{};
    //deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    //DWORD index = 0;
    //// https://learn.microsoft.com/en-us/windows/win32/api/setupapi/nf-setupapi-setupdienumdeviceinfo
    //while (SetupDiEnumDeviceInfo(pDeviceInfoSet, index, &deviceInfoData) == TRUE)
    //{
    //    std::array<BYTE, 512> buffer{};
    //    DWORD requiredSize{};
    //    // https://learn.microsoft.com/en-us/windows/win32/api/setupapi/nf-setupapi-setupdigetdeviceregistrypropertya
    //    WIN_CHECK(SetupDiGetDeviceRegistryProperty(pDeviceInfoSet,
    //                                               &deviceInfoData,
    //                                               SPDRP_HARDWAREID,
    //                                               nullptr,
    //                                               buffer.data(),
    //                                               buffer.size(),
    //                                               &requiredSize));
    //    ASSERT(requiredSize <= buffer.size(), "Pre allocated buffer size to small.");

    //    std::string str{ std::begin(buffer), std::end(buffer) };
    //    LOG_INFO_FMT("Hardware info: {}", str.c_str());
    //    ++index;
    //}
    //DWORD error = GetLastError();
    //ASSERT(error == ERROR_NO_MORE_ITEMS, "There was an error when enumerating devices..");


    //if (pDeviceInfoSet != INVALID_HANDLE_VALUE)
    //{
    //    // https://learn.microsoft.com/en-us/windows/win32/api/setupapi/nf-setupapi-setupdidestroydeviceinfolist
    //    SetupDiDestroyDeviceInfoList(pDeviceInfoSet);
    //}

    return 0;
    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////


    sys::CSystem system{};

    auto wc = security::CWolfCrypt::instance();
    validate_app_directory();
    process_cmd_line_args(argc, argv);

    auto scanner = ble::make_scanner<ble::CScanner>();

    // SDL window and input must be called on the same thread
    gfx::CWindow window{ "Some title", 1'280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY };
    gfx::CRenderer renderer{ window };
    gui::CGui gui{};

    CServer server{};
    auto& deviceList = gui.emplace<gui::CDeviceList>(scanner, server);
    auto& rssiPlot = gui.emplace<gui::CRSSIPlot>(10u, make_rssi_demander(server, window));


    bool exit = false;
    while (!exit)
    {
        common::CStopWatch timer{};

        if (server.connected())
        {
            int8_t median = rssiPlot.rssi_median();
            if (median < -70)
            {
                LOG_INFO("RSSI median is too low - COWABUNGA TIME");
                //sys::cowabunga();
                //      cowabunga();
            }
        }
        else
        {
            if (!server.is_authenticated())
            {
                if (!scanner.scanning())
                {
                    deviceList.recreate_list();
                    rssiPlot = gui::CRSSIPlot{ 10u, make_rssi_demander(server, window) };
                }

                static uint32_t ab = 0;
                if ((ab % 150) == 0)
                {
                    LOG_INFO("No authenticated server - begin cowabunga timer");
                }
                ++ab;
            }
        }


        renderer.begin_frame();

        window.process_events(&exit);

        gui.push();

        renderer.end_frame();


        // frame_time_target(&timer, FPS_120);
        // Throttle application loop to 120 fps.
        double target = 8.333333;
        double timeToWait = target - timer.lap<double>();
        if (timeToWait > 0.0)
        {
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(timeToWait));
        }
    }

    common::coroutine_manager_instance().wait_for_all();

    return EXIT_SUCCESS;
}
