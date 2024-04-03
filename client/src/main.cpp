// Wolfcrypt must be included BEFORE windows.h
#include "security/CHash.hpp"
#include "security/sha.hpp"
#include "security/CWolfCrypt.hpp"
#include "security/CRandom.hpp"
#include "security/ecc_key.hpp"

#include "defines.hpp"
#include "gfx/CRenderer.hpp"
#include "gui/CGui.hpp"


#include <winrt/Windows.Foundation.h>


#include "bluetoothLE/Scanner.hpp"
//#include "bluetoothLE/windows/CDevice.hpp"
#include "bluetoothLE/Device.hpp"
#include "system/System.hpp"
#include "system/TrayIcon.hpp"
#include "common/CStopWatch.hpp"


namespace
{
constexpr std::string_view CLIENT_PUB_NAME = "client_public.der";
constexpr std::string_view CLIENT_PRIV_NAME = "client_private.der";
constexpr std::string_view SERVER_PUB_NAME = "server_public.der";
constexpr std::string_view SERVER_PRIV_NAME = "server_private.der";


void validate_app_directory()
{
    auto validate_app_directory = [](const std::filesystem::path& appDirectory)
    {
        if(!std::filesystem::exists(appDirectory))
            std::filesystem::create_directory(appDirectory);
        
        ASSERT_FMT(std::filesystem::is_directory(appDirectory), "Expected {} to be a directory.", appDirectory.string());
        return sys::key_directory();
    };
    auto validate_key_directory = [](const std::filesystem::path& keyLocation)
    {
        if(!std::filesystem::exists(keyLocation))
            std::filesystem::create_directory(keyLocation);
        
        ASSERT_FMT(std::filesystem::is_directory(keyLocation), "Expected {} to be a directory.", keyLocation.string());
        return std::expected<std::filesystem::path, std::string>{};
    };
    auto log_failure = [](const std::string& err)
    {
        LOG_FATAL_FMT("Could not validate Application Directory because key location could not be retrieved. Reason: {}", err);
        return std::expected<std::filesystem::path, std::string>{};
    };
    [[maybe_unused]] auto result = sys::application_directory()
            .and_then(validate_app_directory)
            .and_then(validate_key_directory)
            .or_else(log_failure);
}
[[nodiscard]] auto make_save_invokable(std::string_view filename)
{
    return [filename = std::filesystem::path{ filename }](std::vector<byte>&& key)
    {
        std::expected<std::filesystem::path, std::string> expected = sys::key_directory();
        if(expected)
        {
            std::filesystem::path filepath = expected->string() / filename;
            std::fstream file{ filepath, std::ios::out | std::ios::binary | std::ios::trunc };
            
            static_assert(alignof(const char) == alignof(decltype(*(key.data()))));
            file.write(reinterpret_cast<const char*>(key.data()), key.size());
            return true;
        }
        else
        {
            LOG_ERROR_FMT("Could not retrieve key location. Failed with: \"{}\"", expected.error());
            return false;
        }
    };
}
[[nodiscard]] auto make_load_invokable(std::string_view filename)
{
    return [filename = std::filesystem::path{ filename }]() -> std::expected<std::vector<byte>, std::string>
    {
        std::expected<std::filesystem::path, std::string> expected = sys::key_directory();
        if(expected)
        {
            std::filesystem::path filepath = expected->string() / filename;
            std::fstream file{ filepath, std::ios::in | std::ios::binary | std::ios::ate };
            if(file.is_open())
            {
                try
                {
                    std::expected<std::vector<byte>, std::string> data{};
                    std::streamsize size = file.tellg();
                    data->resize(size);
                    file.seekg(0);
                    
                    static_assert(alignof(const char) == alignof(decltype(*(data->data()))));
                    file.read(reinterpret_cast<char*>(data->data()), size);
                    
                    return data;
                }
                catch (const std::ios::failure& err)
                {
                    return std::unexpected(std::format("Exception thrown when trying to read file: \"{}\"", err.what()));
                }
            }
            else
            {
                return std::unexpected(std::format("Could not open file: \'{}\'", filename.string()));
            }
        }
        else
        {
            return std::unexpected(std::format("Could not retrieve key location. Failed with: \"{}\"", expected.error()));
        }
    };
}
std::tuple<security::CEccPublicKey, security::CEccPrivateKey> make_ecc_keys()
{
    security::CRandom rng = security::CRandom::make_rng().value();
    security::CEccKeyPair keyPair{ rng };
    
    return { keyPair.public_key(), keyPair.private_key() };
}
void save_ecc_keys(security::CEccPublicKey& pub, security::CEccPrivateKey& priv, std::string_view pubkey, std::string_view privkey)
{
    if(!pub.write_to_disk(make_save_invokable(pubkey)))
    {
        LOG_FATAL("Could not save public key to disk!");
    }
    if(!priv.write_to_disk(make_save_invokable(privkey)))
    {
        LOG_FATAL("Could not save private key to disk!");
    }
    
    auto restrict_private_key = [privkey](const std::filesystem::path& keyLocation)
    {
        sys::restrict_file_permissions(keyLocation / std::filesystem::path{ privkey });
        return std::expected<std::filesystem::path, std::string>{};
    };
    auto log_failure =[](const std::string& err)
    {
        LOG_ERROR_FMT("Could not set private key to admin owned - "
                      "because filepath to key location could not be retrieved: \"{}\"", err);
        return std::expected<std::filesystem::path, std::string>{};
    };
    
    [[maybe_unused]] auto result = sys::key_directory()
            .and_then(restrict_private_key)
            .or_else(log_failure);
}
void process_cmd_line_args(int argc, char** argv)
{
    if (argc < 1)
        return;
    
    for (int32_t i = 1; i < argc; ++i)
    {
        std::string_view arg{ argv[i] };
        if(arg == "--make-keys")
        {
            {
                auto[pubKey, privKey] = make_ecc_keys();
                save_ecc_keys(pubKey, privKey, SERVER_PUB_NAME, SERVER_PRIV_NAME);
            }
            // Client
            {
                auto[pubKey, privKey] = make_ecc_keys();
                save_ecc_keys(pubKey, privKey, CLIENT_PUB_NAME, CLIENT_PRIV_NAME);
            }
        }
        if(arg == "--print-keys")
        {
            auto print_key = [](std::string_view keyName)
            {
                return [keyName](const std::vector<byte>& key)
                {
                    std::stringstream sstream{};
                    for(auto&& byte : key)
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
            
            static constexpr std::array<std::string_view, 4u> keyNames =
                    { CLIENT_PUB_NAME, CLIENT_PRIV_NAME, SERVER_PUB_NAME, SERVER_PRIV_NAME };
            for(auto&& keyName : keyNames)
            {
                auto load_key = make_load_invokable(keyName);
                [[maybe_unused]] auto result = load_key()
                        .and_then(print_key(keyName))
                        .or_else(log_failure);
            }
        }
    }
}
[[nodiscard]] auto time_limited_scan(ble::CScanner& scanner, std::chrono::seconds seconds)
{
    return [&scanner, seconds]()
    {
        scanner.begin_scan();
        std::this_thread::sleep_for(seconds);
        scanner.end_scan();
    };
}
}   // namespace



winrt::fire_and_forget query_device(uint64_t bluetoothAddress)
{
    ble::CDevice device = co_await ble::make_device<ble::CDevice>(bluetoothAddress);
    
    ble::UUID whoami = ble::BaseUUID;
    whoami.custom = ble::ID_SERVICE_WHOAMI;
    
    auto services = device.services();
    auto iter = services.find(whoami);
    if(iter != std::end(services))
    {
        const ble::CService& service = iter->second;
        ble::UUID characteristicUuid = ble::BaseUUID;
        characteristicUuid.custom = ble::ID_CHARS_SERVER_AUTH;
        
        auto result = service.characteristic(characteristicUuid);
        if(result)
        {
            LOG_INFO("Reading characteristic value!");
            const ble::CCharacteristic* pCharacteristic = result.value();
            auto data = co_await pCharacteristic->read_value();
            if(data)
            {
                std::cout << "\nPrinting raw bytes: ";
                for(uint8_t byte : *data)
                    std::cout << byte;
                std::cout << std::endl;
            }
            else
            {
                LOG_ERROR("characteristic.read_value() returned no data.");
            }
        }
        else
        {
            LOG_ERROR("Failed to find characteristic");
        }
    }
    else
    {
        LOG_ERROR("Unable to find service with given UUID");
    }
}


void test_ecc_sign()
{
    security::CRandom rng = security::CRandom::make_rng().value();
    security::CEccKeyPair keyPair{ rng };
    security::CEccPublicKey pubKey = keyPair.public_key();
    security::CEccPrivateKey privKey = keyPair.private_key();
    
    //security::CEccPublicKey pubKey2{ std::move(pubKey) };
    security::CEccPublicKey pubKey2 = std::move(pubKey);
    security::CEccPrivateKey privKey2 { std::move(privKey) };
    
    privKey2.write_to_disk(make_save_invokable("PRIVATE_KEY"));
    pubKey2.write_to_disk(make_save_invokable("PUBLIC_KEY"));
    
    sys::restrict_file_permissions("C:\\Users\\qwerty\\Desktop\\PRIVATE_KEY");
    
    std::optional<security::CEccPublicKey> pubKey3 = security::make_ecc_key<security::CEccPublicKey>(make_load_invokable("PUBLIC_KEY"));
    std::optional<security::CEccPrivateKey> privKey3 = security::make_ecc_key<security::CEccPrivateKey>(make_load_invokable("PRIVATE_KEY"));
    
    
    const char* msg = "Very nice message";
    security::CHash<security::Sha2_256> hash{ msg };
    std::vector<byte> signature = privKey3->sign_hash(rng, hash);
    bool verified = pubKey2.verify_hash(signature, hash);
    if (verified) {
        std::printf("\nSignature Verified Successfully.\n");
    } else {
        std::printf("\nFailed to verify Signature.\n");
    }
}


int main(int argc, char** argv)
{
    tf::Executor& executor = sys::executor();
    
    sys::CSystem system{};
    
    executor.silent_async([argc, argv]()
    {
        auto wc = security::CWolfCrypt::instance();
        validate_app_directory();
        process_cmd_line_args(argc, argv);
    });
    
    auto scanner = ble::make_scanner<ble::CScanner>();
    executor.silent_async(time_limited_scan(scanner, std::chrono::seconds(1)));
    
    
    gfx::CWindow window{ "Some title", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY };
    gfx::CRenderer renderer{ window, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED };
    gui::CGui gui{};
    
    
    bool exit = false;
    while (!exit)
    {
        renderer.begin_frame();
        
        window.process_events(&exit);
        gui.push();
        
        renderer.end_frame();
    }
    
    return EXIT_SUCCESS;
    
    
    
    
    
    
    
    
    //test_ecc_sign();
    
    
    
    //auto result = security::CWolfCrypt::instance();
    //sys::CSystem system{};
    
    //auto scanner = ble::make_scanner<ble::CScanner>();
    //scanner.begin_scan();
    //
    //size_t numDevices = scanner.num_devices().load();
    //scanner.num_devices().wait(numDevices);
    //
    //std::vector<ble::DeviceInfo> infos = scanner.found_devices();
    //if(!infos.empty())
    //{
    //    for(const auto& info : infos)
    //    {
    //        LOG_INFO_FMT("DeviceInfo in cache.\nAddress: {}\nAddress Type: {}",
    //                     ble::hex_addr_to_str(info.address.value()),
    //                     ble::address_type_to_str(info.addressType));
    //
    //        query_device(info.address.value());
    //    }
    //}
    //
    //tf::Executor executor{};
    
    //executor.silent_async([&scanner]()
    //{
    //    while(true)
    //    {
    //        std::vector<ble::DeviceInfo> infos = scanner.found_devices();
    //        if(!infos.empty())
    //        {
    //            for(const auto& info : infos)
    //            {
    //                LOG_INFO_FMT("DeviceInfo in cache.\nAddress: {}\nAddress Type: {}",
    //                             ble::hex_addr_to_str(info.address.value()),
    //                             ble::address_type_to_str(info.addressType));
    //
    //                query_device(info.address.value());
    //            }
    //            break;
    //        }
    //
    //        std::this_thread::sleep_for(std::chrono::seconds(1));
    //    }
    //});
    
    
    

    //try
    //{
    //    process_cmd_line_args(argc, argv);
    //
//
    //
    //    gfx::CWindow window{ "Some title", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY };
    //    gfx::CRenderer renderer{ window, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED };
    //    gui::CGui gui{};
    //
    //
    //
    //
    //    tf::Executor executor{};
    //    executor.silent_async(time_limited_scan(scanner, std::chrono::seconds(10)));
    //
    //    bool exit = false;
    //    while (!exit)
    //    {
    //        renderer.begin_frame();
//
    //        window.process_events(&exit);
    //        gui.push();
//
    //        renderer.end_frame();
    //    }
    //
    //
    //    return EXIT_SUCCESS;
    //}
    //catch(const exception::fatal_error& err)
    //{
    //    LOG_ERROR_FMT("Fatal exception: {}", err.what());
    //    return EXIT_FAILURE;
    //}
}