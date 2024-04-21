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

#include <winrt/Windows.Foundation.h>

#include "bluetoothLE/Scanner.hpp"
#include "bluetoothLE/Device.hpp"
#include "gui/CDeviceList.hpp"
#include "common/ble_services.hpp"


namespace
{
constexpr std::string_view CLIENT_PUBLIC_KEY_NAME = "client_public.der";
constexpr std::string_view CLIENT_PRIVATE_KEY_NAME = "client_private.der";
constexpr std::string_view SERVER_PUBLIC_KEY_NAME = "server_public.der";
constexpr std::string_view SERVER_PRIVATE_KEY_NAME = "server_private.der";


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
[[nodiscard]] auto make_invokable_load_file(std::string_view filename)
{
    return [filename = std::filesystem::path{ filename }]() -> std::expected<std::vector<security::byte>, std::string>
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
                    std::expected<std::vector<security::byte>, std::string> data{};
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
template<typename key_t>
requires std::same_as<key_t, security::CEccPublicKey> || std::same_as<key_t, security::CEccPrivateKey>
[[nodiscard]] std::unique_ptr<key_t> load_key(std::string_view keyName)
{
    auto loadKey = make_invokable_load_file(keyName);
    
    std::optional<key_t> key = security::make_ecc_key<key_t>(loadKey);
    if(!key)
    {
        LOG_FATAL_FMT("Cannot continue because \"{}\" could not be loaded.", keyName);
    }
    
    return std::make_unique<key_t>(std::move(*key));
}
[[nodiscard]] auto make_invokable_save_file(std::string_view filename)
{
    return [filename = std::filesystem::path{ filename }](std::vector<security::byte>&& key)
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
std::tuple<security::CEccPublicKey, security::CEccPrivateKey> make_ecc_keys()
{
    security::CRandom rng = security::CRandom::make_rng().value();
    security::CEccKeyPair keyPair{ rng };
    
    return { keyPair.public_key(), keyPair.private_key() };
}
void save_ecc_keys(security::CEccPublicKey& pub, security::CEccPrivateKey& priv, std::string_view pubkey, std::string_view privkey)
{
    if(!pub.write_to_disk(make_invokable_save_file(pubkey)))
    {
        LOG_FATAL("Could not save public key to disk!");
    }
    if(!priv.write_to_disk(make_invokable_save_file(privkey)))
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
                save_ecc_keys(pubKey, privKey, SERVER_PUBLIC_KEY_NAME, SERVER_PRIVATE_KEY_NAME);
            }
            {
                auto[pubKey, privKey] = make_ecc_keys();
                save_ecc_keys(pubKey, privKey, CLIENT_PUBLIC_KEY_NAME, CLIENT_PRIVATE_KEY_NAME);
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
                    { CLIENT_PUBLIC_KEY_NAME, CLIENT_PRIVATE_KEY_NAME, SERVER_PUBLIC_KEY_NAME, SERVER_PRIVATE_KEY_NAME };
            for(auto&& keyName : keyNames)
            {
                auto load_key = make_invokable_load_file(keyName);
                [[maybe_unused]] auto result = load_key()
                        .and_then(print_key(keyName))
                        .or_else(log_failure);
            }
        }
    }
}
template<typename sha_t>
[[nodiscard]] uint8_t get_sha_type_id()
{
    if constexpr (std::same_as<sha_t, security::Sha2_224>)
        return std::to_underlying(ble::HashType::Sha2_224);
    else if constexpr (std::same_as<sha_t, security::Sha2_256>)
        return std::to_underlying(ble::HashType::Sha2_256);
    else if constexpr (std::same_as<sha_t, security::Sha2_384>)
        return std::to_underlying(ble::HashType::Sha2_384);
    else if constexpr (std::same_as<sha_t, security::Sha2_512>)
        return std::to_underlying(ble::HashType::Sha2_512);
    else if constexpr (std::same_as<sha_t, security::Sha3_224>)
        return std::to_underlying(ble::HashType::Sha3_224);
    else if constexpr (std::same_as<sha_t, security::Sha3_256>)
        return std::to_underlying(ble::HashType::Sha3_256);
    else if constexpr (std::same_as<sha_t, security::Sha3_384>)
        return std::to_underlying(ble::HashType::Sha3_384);
    else
        return std::to_underlying(ble::HashType::Sha3_512);
}
[[nodiscard]] std::vector<byte> generate_random_block(security::CRandom& rng)
{
    static std::random_device rd{};
    static std::mt19937_64 generator{ rd() };
    static std::uniform_int_distribution<size_t> distribution{ 64u, 96u };
    size_t blockSize = distribution(generator);

    std::expected<std::vector<byte>, security::CRandom::Error> expected = rng.generate_block(blockSize);
    if (!expected)
    {
        LOG_ERROR_FMT("Failed to generate random block of data of size: {}", blockSize);
        return { 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD, 0xDE, 0xAD };
    }

    return std::move(*expected);
}
[[nodiscard]] std::vector<byte> insert_random_data_block(std::vector<byte>& packet, const std::vector<byte>& randomBlock)
{
    static constexpr ble::DemandRSSIHeader HEADER = ble::header_whereami_demand_rssi();

    packet[HEADER.randomDataOffset] = sizeof(decltype(HEADER));
    packet[HEADER.randomDataSize] = common::assert_down_cast<uint8_t>(randomBlock.size());

    std::span<uint8_t> packetRandomBlock{ std::begin(packet) + packet[HEADER.randomDataOffset], packet[HEADER.randomDataSize] };
    ASSERT(packetRandomBlock.size() == randomBlock.size(), "Buffer size mismatch when calling memcpy");
    size_t bytesToCopy = randomBlock.size() < packetRandomBlock.size() ? randomBlock.size() : packetRandomBlock.size();
    std::memcpy(packetRandomBlock.data(), randomBlock.data(), bytesToCopy);

    return packet;
}
template<typename sha_t>
    requires security::HashAlgorithm<sha_t>
[[nodiscard]] std::vector<byte> insert_hash(std::vector<byte>& packet, const security::CHash<sha_t>& hash)
{
    static constexpr ble::DemandRSSIHeader HEADER = ble::header_whereami_demand_rssi();

    packet[HEADER.hashOffset] = sizeof(decltype(HEADER)) + packet[HEADER.randomDataSize];
    packet[HEADER.hashSize] = common::assert_down_cast<uint8_t>(hash.size());

    std::span<uint8_t> packetHashBlock{ std::begin(packet) + packet[HEADER.hashOffset], packet[HEADER.hashSize] };
    ASSERT(packetHashBlock.size() == hash.size(), "Buffer size mismatch when calling memcpy");
    size_t bytesToCopy = hash.size() < packetHashBlock.size() ? hash.size() : packetHashBlock.size();
    std::memcpy(packetHashBlock.data(), hash.data(), bytesToCopy);

    return packet;
}
[[nodiscard]] std::vector<byte> insert_signature(std::vector<byte>& packet, const std::vector<byte>& signature)
{
    static constexpr ble::DemandRSSIHeader HEADER = ble::header_whereami_demand_rssi();

    packet[HEADER.signatureOffset] = sizeof(decltype(HEADER)) + packet[HEADER.randomDataSize] + packet[HEADER.hashSize];
    packet[HEADER.signatureSize] = common::assert_down_cast<uint8_t>(signature.size());

    std::span<uint8_t> packetSignatureBlock{ std::begin(packet) + packet[HEADER.signatureOffset], packet[HEADER.signatureSize] };
    ASSERT(packetSignatureBlock.size() == signature.size(), "Buffer size mismatch when calling memcpy");
    size_t bytesToCopy = signature.size() < packetSignatureBlock.size() ? signature.size() : packetSignatureBlock.size();
    std::memcpy(packetSignatureBlock.data(), signature.data(), bytesToCopy);

    return packet;
}
[[nodiscard]] std::vector<byte> make_packet_demand_rssi(security::CEccPrivateKey* pPrivateKey)
{
    using sha_type = security::Sha2_256;
    static std::expected<security::CRandom, security::CRandom::Error> expected = security::CRandom::make_rng();
    if (!expected)
    {
        LOG_FATAL("Failed to create cryptographic rng generator");
    }

    security::CRandom& rng = *expected;
    std::vector<byte> randomBlock = generate_random_block(rng);
    security::CHash<sha_type> hash{ randomBlock };
    std::vector<byte> signature = pPrivateKey->sign_hash(rng, hash);

    static constexpr ble::DemandRSSIHeader HEADER = ble::header_whereami_demand_rssi();
    const size_t PACKET_SIZE = sizeof(decltype(HEADER)) + randomBlock.size() + hash.size() + signature.size();
    ASSERT(PACKET_SIZE < 216, "Max packet size for BLE is around 216 bytes - give or take");

    std::vector<byte> packet{};
    packet.resize(PACKET_SIZE);
    packet[HEADER.hashType] = get_sha_type_id<typename decltype(hash)::hash_type>();
    packet = insert_random_data_block(packet, randomBlock);
    packet = insert_hash(packet, hash);
    packet = insert_signature(packet, signature);

    return packet;
}
sys::fire_and_forget_t try_demand_rssi(
    gfx::CWindow& wnd, 
    CAuthenticator& authenticator, 
    const ble::CCharacteristic& characteristic,
    std::vector<byte> packet)
{
    static constexpr int32_t MAX_ATTEMPS = 3;
    int32_t attempt{};
    do
    {
        auto communicationStatus = co_await characteristic.write_data(packet);

        LOG_INFO_FMT("Write status: {}", ble::gatt_communication_status_to_str(communicationStatus));
        switch (communicationStatus)
        {
        case ble::CommunicationStatus::unreachable:
        {
            authenticator.deauth();
            wnd.popup_warning("Unreachable", "Could not demand RSSI value from server");
            [[fallthrough]];
        }
        case ble::CommunicationStatus::success:
        {
            attempt = MAX_ATTEMPS;
            break;
        }
        case ble::CommunicationStatus::accessDenied:
        {
            LOG_WARN("Could not write to WhereAmI's demand RSSI characteristic - Access Was Denied");
            wnd.popup_warning("Access Denied", "Could not demand RSSI value from server");
            [[fallthrough]];
        }
        case ble::CommunicationStatus::protocolError:
        {
            ++attempt;
        }
        }
    } while (attempt < MAX_ATTEMPS);
}
}   // namespace


[[nodiscard]] std::optional<const ble::CCharacteristic*> find_characteristic_demand_rssi(const ble::CService* pService)
{
    return pService->characteristic(ble::uuid_characteristic_whereami_demand_rssi());
}
[[nodiscard]] auto demand_rssi(gfx::CWindow& wnd, CAuthenticator& authenticator, security::CEccPrivateKey* pPrivateKey)
{
    return [&wnd, &authenticator, pPrivateKey](const ble::CCharacteristic* pCharacteristic) 
    {
        std::vector<byte> packet = make_packet_demand_rssi(pPrivateKey);
        try_demand_rssi(wnd, authenticator, *pCharacteristic, packet);

        return std::optional<const ble::CCharacteristic*>{ pCharacteristic };
    };
}
sys::fire_and_forget_t try_connect_to_server(
    gfx::CWindow& wnd, 
    std::optional<ble::CDevice>& outDevice, 
    uint64_t address,
    std::atomic<bool>& attemptingConnection)
{
    {
        bool expected = false;
        while (!attemptingConnection.compare_exchange_strong(expected, true)) {};
    }
    LOG_INFO("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");

    std::expected<ble::CDevice, ble::CDevice::Error> expected = co_await ble::CDevice::make(address);
    if (expected)
    {
        outDevice.emplace(std::move(*expected));
        int a = 10;
    }
    else
    {
        wnd.popup_warning("Connection Attempt Failed", "Could not connect to authenticated mac address.");
    }

    {
        bool expected = true;
        while (!attemptingConnection.compare_exchange_strong(expected, false)) {};
    }
}


int main(int argc, char** argv)
{
    sys::CSystem system{};

    auto wc = security::CWolfCrypt::instance();
    validate_app_directory();
    process_cmd_line_args(argc, argv);

    std::unique_ptr<security::CEccPublicKey> pPublicKey = load_key<security::CEccPublicKey>(CLIENT_PUBLIC_KEY_NAME);
    std::unique_ptr<security::CEccPrivateKey> pPrivateKey = load_key<security::CEccPrivateKey>(CLIENT_PRIVATE_KEY_NAME);
    std::unique_ptr<security::CEccPublicKey> pServerKey = load_key<security::CEccPublicKey>(SERVER_PUBLIC_KEY_NAME);

    auto scanner = ble::make_scanner<ble::CScanner>();
    
    // SDL window and input must be called on the same thread
    gfx::CWindow window{ "Some title", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY };
    gfx::CRenderer renderer{ window, SDL_RENDERER_PRESENTVSYNC };
    gui::CGui gui{};
 
    CAuthenticator authenticator{ pServerKey.get() };
    gui::CDeviceList& deviceList = gui.emplace<gui::CDeviceList>(scanner, authenticator);
    gui::CRSSIPlot& rssiPlot = gui.emplace<gui::CRSSIPlot>(30u);

    std::optional<ble::CDevice> device = std::nullopt;
    
    bool exit = false;
    while (!exit)
    {
        static float val = 0.0f;
        static float b = 0.0f;
        static uint64_t frame = 0u;
        if(frame++ % 60 == 0)
        {
            b = b + 0.35f;
            val = std::sin(b);
            rssiPlot.add_rssi_value(val);
        }
        
        static common::CStopWatch<std::chrono::seconds> demandRssiTimer{};
        if (demandRssiTimer.lap<float>() > static_cast<float>(std::chrono::seconds(5).count()))
        {
            if (device && device->connected())
            {
                device->service(ble::uuid_service_whereami())
                .and_then(find_characteristic_demand_rssi)
                .and_then(demand_rssi(window, authenticator, pPrivateKey.get()));
                
                demandRssiTimer.reset();
            }
            else
            {
                if (authenticator.server_identified())
                {
                    device = authenticator.server();
                }
                else
                {
                    // TODO::
                    if (!scanner.scanning())
                    {
                        deviceList.recreate_list();
                    }

                    static int ab = 0;
                    if ((ab % 50) == 0)
                    {
                        LOG_INFO("No authenticated server - begin cowabunga timer");
                        ++ab;
                    }
                }
            }
        }
        
        
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