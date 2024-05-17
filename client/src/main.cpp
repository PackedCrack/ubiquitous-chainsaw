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
#include "common/client_common.hpp"

#include "common/serial_communication.hpp"
#include "system/SerialCommunication.hpp"
#include "common/CCoroutineManager.hpp"
// clang-format off


// clang-format on
namespace
{
constexpr double FPS_120 = 8.333333;
constexpr double FPS_144 = 6.944444;
template<typename duration_t>
void frame_time_target(const common::CStopWatch<duration_t>& timer, double targetTime)
{
    double timeToWait = targetTime - timer.lap<double>();
    if (timeToWait > 0.0)
    {
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(timeToWait));
    }
}
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
                    return std::expected<std::vector<byte>, security::ErrorMakeEccKey>{};
                };
            };
            auto log_failure = [](security::ErrorMakeEccKey err)
            {
                LOG_ERROR_FMT("Could not print key contents. Reason: \"{}\"", std::to_underlying(err));

                return std::expected<std::vector<byte>, security::ErrorMakeEccKey>{};
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
[[nodiscard]] bool send_key(common::KeyType keyType, const std::vector<uint8_t>& keyData)
{
    std::expected<sys::CSerialCommunication, sys::ErrorSerialCom> serialComm = sys::open_serial_communication();
    if (serialComm)
    {
        common::KeyTransferHeader header{ .keyType = std::to_underlying(keyType),
                                          .keySize = common::assert_down_cast<uint8_t>(keyData.size()) };

        std::array<uint8_t, 1> keyType{ header.keyType };
        int32_t bytesWritten = serialComm->write(keyType);

        std::array<uint8_t, 1> keySize{ header.keySize };
        bytesWritten = bytesWritten + serialComm->write(keySize);

        bytesWritten = bytesWritten + serialComm->write(keyData);

        return bytesWritten == keyData.size() + sizeof(common::KeyTransferHeader) ? true : false;
    }
    else
    {
        // TODO: do something with some errors - DeviceNotFound should show helpful message in gui
        LOG_ERROR_FMT("Failed to sent key. Reason: {}", sys::err_serial_com_to_str(serialComm.error()));
    }

    return false;
}
[[nodiscard]] gui::CGui make_gui(gfx::CWindow& window, CServer& server)
{
    auto generateKeysAction = [&server]() mutable
    {
        // wait for coroutines
        common::coroutine_manager_instance().wait_for_all();
        // make keys
        {
            auto [pubKey, privKey] = make_ecc_keys();
            save_ecc_key(pubKey, SERVER_PUBLIC_KEY_NAME);

            {
                std::stringstream sstream{};
                for (auto&& byte : pubKey.to_der())
                {
                    sstream << std::setw(2) << std::setfill('0') << std::hex << static_cast<int32_t>(byte);
                }
                LOG_INFO_FMT("Server Public Key: {}", sstream.str());
            }
            {
                std::stringstream sstream{};
                for (auto&& byte : privKey.to_der())
                {
                    sstream << std::setw(2) << std::setfill('0') << std::hex << static_cast<int32_t>(byte);
                }
                LOG_INFO_FMT("Server Private Key: {}", sstream.str());
            }

            if (!send_key(common::KeyType::serverPublic, pubKey.to_der()))
            {
                LOG_ERROR("Failed to send Servers Public Key");
            }
            if (!send_key(common::KeyType::serverPrivate, privKey.to_der()))
            {
                LOG_ERROR("Failed to send Servers Private Key");
            }


            //save_ecc_keys(pubKey, privKey, SERVER_PUBLIC_KEY_NAME, SERVER_PRIVATE_KEY_NAME);
        }
        {
            auto [pubKey, privKey] = make_ecc_keys();
            save_ecc_key(pubKey, CLIENT_PUBLIC_KEY_NAME);
            save_ecc_key(privKey, CLIENT_PRIVATE_KEY_NAME);

            {
                std::stringstream sstream{};
                for (auto&& byte : pubKey.to_der())
                {
                    sstream << std::setw(2) << std::setfill('0') << std::hex << static_cast<int32_t>(byte);
                }
                LOG_INFO_FMT("Client Public Key: {}", sstream.str());
            }
            if (!send_key(common::KeyType::clientPublic, pubKey.to_der()))
            {
                LOG_ERROR("Failed to send Clients Public Key");
            }

            //save_ecc_keys(pubKey, privKey, CLIENT_PUBLIC_KEY_NAME, CLIENT_PRIVATE_KEY_NAME);
        }
        // send keys
        // recreate key owning widgets
        server.reload_public_key();
    };
    auto eraseKeysAction = [&window, &server](gui::CRSSIPlot& rssiPlot, gui::CDeviceList& deviceList) mutable
    {
        // wait for coroutines
        common::coroutine_manager_instance().wait_for_all();
        // delete keys
        erase_stored_ecc_keys();
        // recreate key owning widgets
        server.reload_public_key();
        rssiPlot = gui::CRSSIPlot{ make_rssi_demander(server, window) };
        deviceList.clear_list();
    };

    return gui::CGui{ std::move(generateKeysAction), std::move(eraseKeysAction) };
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
namespace
{}    // namespace
int main(int argc, char** argv)
{
    /*std::expected<sys::CSerialCommunication, sys::ErrorSerialCom> expected = sys::open_serial_communication();

    sys::CSerialCommunication& serial = *expected;

    LOG_INFO("Waiting until go..");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    LOG_INFO("GO!");
    {
        std::array<uint8_t, 1> keyType{ 1 };
        uint32_t bytesWritten = serial.write(keyType);
        LOG_INFO_FMT("Wrote {} bytes", bytesWritten);
    }
    {
        std::array<uint8_t, 1> keySize{ 14 };
        uint32_t bytesWritten = serial.write(keySize);
        LOG_INFO_FMT("Wrote {} bytes", bytesWritten);
    }
    {
        std::array<uint8_t, 14> keyData{ 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78 };
        uint32_t bytesWritten = serial.write(keyData);
        LOG_INFO_FMT("Wrote {} bytes", bytesWritten);
    }

    return 0;*/
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

    CServer server{};
    gui::CGui gui = make_gui(window, server);


    auto& deviceList = gui.emplace<gui::CDeviceList>(scanner, server);
    auto& rssiPlot = gui.emplace<gui::CRSSIPlot>(make_rssi_demander(server, window));


    bool exit = false;
    while (!exit)
    {
        common::CStopWatch timer{};

        if (keys_exists())
        {
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
                        rssiPlot = gui::CRSSIPlot{ make_rssi_demander(server, window) };
                    }

                    static uint32_t ab = 0;
                    if ((ab % 150) == 0)
                    {
                        LOG_INFO("No authenticated server - begin cowabunga timer");
                    }
                    ++ab;
                }
            }
        }
        else
        {
            // some gui
        }


        renderer.begin_frame();

        window.process_events(&exit);

        gui.push();

        renderer.end_frame();


        // Throttle application loop to 120 fps.
        frame_time_target(timer, FPS_120);
    }

    common::coroutine_manager_instance().wait_for_all();

    return EXIT_SUCCESS;
}
