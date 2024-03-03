#include "defines.hpp"
#include "gfx/CRenderer.hpp"
#include "gfx/SDL_Defines.hpp"
#include "gui/CGui.hpp"

#include "security/CHash.hpp"
#include "security/sha.hpp"
#include "security/CWolfCrypt.hpp"
#include "security/CRandom.hpp"
#include "security/ecc_key.hpp"


#include "wolfssl/options.h"
#include "wolfssl/wolfcrypt/ecc.h"
#include "wolfssl/wolfcrypt/asn.h"
#include "wolfssl/wolfcrypt/random.h"
#include "wolfssl/wolfcrypt/sha256.h"
#include "wolfssl/ssl.h"
#include "wolfssl/wolfcrypt/asn_public.h"
#include "wolfssl/wolfcrypt/signature.h"

#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Foundation.Collections.h>

#include <iostream>
#include <coroutine>
#include "bluetoothLE/CBLEScanner.hpp"
#include "bluetoothLE/windows/CDevice.hpp"
#include "bluetoothLE/windows/SystemAPI.h"

#include "defense/defense_mechanism.hpp"







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



void query_device(uint64_t bluetoothAddress)
{
    ble::win::CDevice device = ble::win::CDevice::make_device(bluetoothAddress);
    while(!device.ready())
    {
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

#include "common/CMutex.hpp"

int main(int argc, char** argv)
{
    //auto result = security::CWolfCrypt::instance();
    
    //test_ecc_sign();
    //return 0;

    
    ASSERT_FMT(0 < argc, "ARGC is {} ?!", argc);
    
    
    ble::win::SystemAPI system{};
    
    ble::CBLEScanner scanner = ble::make_scanner();
    scanner.begin_scan();
    
    
    while(true)
    {
        std::vector<ble::DeviceInfo> infos = scanner.found_devices();
        if(!infos.empty())
        {
            
            for(const auto& info : infos)
            {
                LOG_INFO_FMT("DeviceInfo in cache.\nAddress: {}\nAddress Type: {}",
                             ble::hex_addr_to_str(info.address.value()),
                             ble::address_type_to_str(info.addressType));
    
                query_device(info.address.value());
            }
            break;
        }
    
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    

    try
    {
        process_cmd_line_args(argc, argv);
        

        
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
    }
    catch(const exception::fatal_error& err)
    {
        LOG_ERROR_FMT("Fatal exception: {}", err.what());
        return EXIT_FAILURE;
    }
}