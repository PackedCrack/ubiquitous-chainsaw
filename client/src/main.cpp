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
    [[maybe_unused]] auto result =
        sys::application_directory().and_then(validate_app_directory).and_then(validate_key_directory).or_else(log_failure);
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
}    // namespace
int main(int argc, char** argv)
{
    sys::CSystem system{};

    auto wc = security::CWolfCrypt::instance();
    validate_app_directory();
    process_cmd_line_args(argc, argv);

    auto scanner = ble::make_scanner<ble::CScanner>();

    // SDL window and input must be called on the same thread
    gfx::CWindow window{ "Some title", 1'280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY };
    gfx::CRenderer renderer{ window, SDL_RENDERER_PRESENTVSYNC };
    gui::CGui gui{};

    CServer server{};
    CAuthenticator authenticator{ server };
    auto& deviceList = gui.emplace<gui::CDeviceList>(scanner, authenticator);
    auto& rssiPlot = gui.emplace<gui::CRSSIPlot>(30u, std::make_shared<CRssiDemander>(server, window, std::chrono::seconds(1)));


    bool exit = false;
    while (!exit)
    {
        common::CStopWatch timer{};

        //static float val = 0.0f;
        //static float b = 0.0f;
        //static uint64_t frame = 0u;
        //if (frame++ % 60 == 0)
        //{
        //    b = b + 0.35f;
        //    val = std::sin(b);
        //    rssiPlot.add_rssi_value(val);
        //}

        if (server.connected())
        {
            // int missedAnswers = rssiPlot.missed();
            // if (missedAnswers > 5)
            //      cowabunga();

            float avg = rssiPlot.rssi_avg();
            if (avg < -70.0f)
            {
                LOG_INFO("RSSI avg is too low - COWABUNGA TIME");
            }
            //      cowabunga();

            //std::optional<int8_t> rssi = demander.rssi();
            //server.demand_rssi(window);
        }
        else
        {
            if (!server.is_authenticated())
            {
                // TODO::
                if (!scanner.scanning())
                {
                    LOG_INFO("RE CREATING RSSI DEMANDER");
                    deviceList.recreate_list();
                    rssiPlot = gui::CRSSIPlot{ 30u, std::make_shared<CRssiDemander>(server, window, std::chrono::seconds(1)) };
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
