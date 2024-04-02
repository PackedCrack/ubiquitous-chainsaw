// Taskflow must be included BEFORE windows.h
#include "taskflow/taskflow.hpp"

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


#include "bluetoothLE/CBLEScanner.hpp"
//#include "bluetoothLE/windows/CDevice.hpp"
#include "bluetoothLE/Device.hpp"
#include "system/windows/System.h"
#include "system/windows/CTrayIcon.hpp"



namespace
{
void process_cmd_line_args(int argc, char** argv)
{
    if (argc < 1)
        LOG_FATAL_FMT("Not enough command line arguments given, expected at least 1. But {} was provided", argc);

    std::vector<std::string> arguments{};
    arguments.reserve(static_cast<std::size_t>(argc));
    for (int32_t i = 0; i < argc; ++i)
    {
        arguments.emplace_back(argv[i]);
    }


    for (std::string_view arg : arguments)
    {
        // TODO
    }
}
}   // namespace



winrt::fire_and_forget query_device(uint64_t bluetoothAddress)
{
    LOG_INFO("Before creation");
    ble::CDevice device = co_await ble::make_device<ble::CDevice>(bluetoothAddress);
    LOG_INFO("After creation");
    while(!device.ready())
    {
    }
    
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
            pCharacteristic->read_value();
        }
        else
        {
            if(result.error() == ble::CService::Error::characteristicNotFound)
            {
                LOG_ERROR("Failed to find characteristic");
            }
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
    
    
    const char* msg = "Very nice message";
    security::CHash<security::Sha2_256> hash{ msg };
    std::vector<byte> signature = privKey.sign_hash(rng, hash);
    bool verified = pubKey.verify_hash(signature, hash);
    if (verified) {
        std::printf("\nSignature Verified Successfully.\n");
    } else {
        std::printf("\nFailed to verify Signature.\n");
    }
}

int main(int argc, char** argv)
{
    ASSERT_FMT(0 < argc, "ARGC is {} ?!", argc);
    
    
    auto result = security::CWolfCrypt::instance();
    sys::System system{};
    
    
    LOG_INFO("Before scanner");
    
    ble::CBLEScanner scanner = ble::make_scanner();
    scanner.begin_scan();
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::vector<ble::DeviceInfo> infos = scanner.found_devices();
    if(!infos.empty())
    {
        LOG_INFO("Looping over infos");
        for(const auto& info : infos)
        {
            LOG_INFO_FMT("DeviceInfo in cache.\nAddress: {}\nAddress Type: {}",
                         ble::hex_addr_to_str(info.address.value()),
                         ble::address_type_to_str(info.addressType));
            
            query_device(info.address.value());
        }
        LOG_INFO("Done looping over infos");
    }
    
    tf::Executor executor{};
    
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
    
    
    

    try
    {
        process_cmd_line_args(argc, argv);
        

        
        gfx::CWindow window{ "Some title", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY };
        gfx::CRenderer renderer{ window, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED };
        gui::CGui gui{};
        
        
        
        
        tf::Executor executor{};
        executor.silent_async([&window]() mutable
        {
            for(int i = 0; i < 1; ++i)
            {
                window.popup_warning("Potential Masquerading", "Identified two BLE Servers with the same signed MAC address");
            }
        });
        
        bool exit = false;
        while (!exit)
        {
            renderer.begin_frame();

            window.process_events(&exit);
            gui.push();

            renderer.end_frame();
        }
        
        
        return EXIT_SUCCESS;
    }
    catch(const exception::fatal_error& err)
    {
        LOG_ERROR_FMT("Fatal exception: {}", err.what());
        return EXIT_FAILURE;
    }
}