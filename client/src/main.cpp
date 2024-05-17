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

#include "bluetoothLE/Scanner.hpp"
#include "bluetoothLE/Device.hpp"
#include "gui/CDeviceList.hpp"
#include "CRssiDemander.hpp"
#include "common/client_common.hpp"

#include "common/serial_communication.hpp"
#include "system/SerialCommunication.hpp"
#include "common/CCoroutineManager.hpp"
//
//
//
// Set this to 0 during development unless you want to risk having computer hibernate..
#define COWABUNGA             1
#define HIBERNATION_THRESHOLD -77
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
                                                                           SERVER_PUBLIC_KEY_NAME };
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

        std::array<uint8_t, 1> type{ header.keyType };
        int32_t bytesWritten = serialComm->write(type);

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
        common::coroutine_manager_instance().wait_for_all();

        {
            auto [pubKey, privKey] = make_ecc_keys();
            save_ecc_key(pubKey, SERVER_PUBLIC_KEY_NAME);

            if (!send_key(common::KeyType::serverPublic, pubKey.to_der()))
            {
                LOG_ERROR("Failed to send Servers Public Key");
            }
            if (!send_key(common::KeyType::serverPrivate, privKey.to_der()))
            {
                LOG_ERROR("Failed to send Servers Private Key");
            }
        }
        {
            auto [pubKey, privKey] = make_ecc_keys();
            save_ecc_key(pubKey, CLIENT_PUBLIC_KEY_NAME);
            save_ecc_key(privKey, CLIENT_PRIVATE_KEY_NAME);

            if (!send_key(common::KeyType::clientPublic, pubKey.to_der()))
            {
                LOG_ERROR("Failed to send Clients Public Key");
            }
        }
        server.reload_public_key();
    };
    auto eraseKeysAction = [&window, &server](gui::CRSSIPlot& rssiPlot, gui::CDeviceList& deviceList) mutable
    {
        common::coroutine_manager_instance().wait_for_all();

        erase_stored_ecc_keys();
        server.reload_public_key();
        rssiPlot = gui::CRSSIPlot{ make_rssi_demander(server, window) };
        deviceList.clear_list();
    };

    return gui::CGui{ std::move(generateKeysAction), std::move(eraseKeysAction) };
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
                if (median < HIBERNATION_THRESHOLD && rssiPlot.median_buffer_ratio() > 0.50f)
                {
#if COWABUNGA
                    sys::cowabunga();
                    sys::auto_wakeup_timer(std::chrono::seconds(10));
#else
                    LOG_INFO("RSSI median is too low - COWABUNGA TIME");
#endif
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
