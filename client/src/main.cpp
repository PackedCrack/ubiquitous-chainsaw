#include "defines.hpp"
#include "gfx/CRenderer.hpp"
#include "gfx/SDL_Defines.hpp"
#include "gui/CGui.hpp"

#include "security/hash.hpp"
#include "security/CWolfCrypt.hpp"
#include "security/CRandom.hpp"
#include "security/CEccKey.hpp"


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





winrt::fire_and_forget query_device(uint64_t bluetoothAddress)
{
    using namespace winrt::Windows::Devices::Bluetooth;
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    
    auto bluetoothLeDevice = co_await BluetoothLEDevice::FromBluetoothAddressAsync(bluetoothAddress);
    // Further operations on bluetoothLeDevice
    
    // Discover services because lmfao
    auto servicesResult = co_await bluetoothLeDevice.GetGattServicesAsync();
    
    
    if (servicesResult.Status() == GattCommunicationStatus::Success)
    {
        auto services = servicesResult.Services();
        for(auto&& service : services)
        {
            std::printf("\n\n------------------");
            
            std::printf("\nService UUID: %ws", to_hstring(service.Uuid()).data());
            
            // characteristics
            auto characteristicsResults = co_await service.GetCharacteristicsAsync();
            if(characteristicsResults.Status() == GattCommunicationStatus::Success)
            {
                auto characteristics = characteristicsResults.Characteristics();
                for(auto&& characteristic : characteristics)
                {
                    std::printf("\nCharacteristic UUID: %ws", to_hstring(characteristic.Uuid()).data());
                    // User firendly descrition
                    std::printf("\nUser friendly description: %ws", characteristic.UserDescription().data());
                    
                    // Protection level
                    auto level = characteristic.ProtectionLevel();
                    switch(level)
                    {
                        case GattProtectionLevel::AuthenticationRequired:
                            std::printf("\nProtection level: Authentication Required");
                            break;
                        case GattProtectionLevel::EncryptionAndAuthenticationRequired:
                            std::printf("\nProtection level: Encryption And Authentication Required");
                            break;
                        case GattProtectionLevel::EncryptionRequired:
                            std::printf("\nProtection level: Encryption Required");
                            break;
                        case GattProtectionLevel::Plain:
                            std::printf("\nProtection level: Plain");
                            break;
                    }
                    
                    // Properties
                    auto properties = characteristic.CharacteristicProperties();
                    std::string props{ "\nProperties level:"};
                    if(std::to_underlying(properties) & std::to_underlying(GattCharacteristicProperties::AuthenticatedSignedWrites))
                        props += " Authenticated Signed Writes,";
                    if(std::to_underlying(properties) & std::to_underlying(GattCharacteristicProperties::Broadcast))
                        props += " Broadcast,";
                    if(std::to_underlying(properties) & std::to_underlying(GattCharacteristicProperties::ExtendedProperties))
                        props += " Extended Properties,";
                    if(std::to_underlying(properties) & std::to_underlying(GattCharacteristicProperties::Indicate))
                        props += " Indicate,";
                    if(std::to_underlying(properties) & std::to_underlying(GattCharacteristicProperties::None))
                        props += " None,";
                    if(std::to_underlying(properties) & std::to_underlying(GattCharacteristicProperties::Notify))
                        props += " Notify,";
                    if(std::to_underlying(properties) & std::to_underlying(GattCharacteristicProperties::Read))
                        props += " Read,";
                    if(std::to_underlying(properties) & std::to_underlying(GattCharacteristicProperties::ReliableWrites))
                        props += " Reliable Writes,";
                    if(std::to_underlying(properties) & std::to_underlying(GattCharacteristicProperties::WritableAuxiliaries))
                        props += " Writable Auxiliaries,";
                    if(std::to_underlying(properties) & std::to_underlying(GattCharacteristicProperties::Write))
                        props += " Write,";
                    if(std::to_underlying(properties) & std::to_underlying(GattCharacteristicProperties::WriteWithoutResponse))
                        props += " Write Without Response,";
                    
                    if(props.ends_with(','))
                        props.pop_back();
                    
                    std::printf("%s", props.c_str());
                    
                    
                    
                    // Descriptors
                    
                    auto descriptorResults = co_await characteristic.GetDescriptorsAsync();
                    if(descriptorResults.Status() == GattCommunicationStatus::Success)
                    {
                        auto descriptors = descriptorResults.Descriptors();
                        for(auto&& descriptor : descriptors)
                        {
                            std::printf("\nDescriptor UUID: %ws", to_hstring(descriptor.Uuid()).data());
                            
                            auto protLevel = descriptor.ProtectionLevel();
                            switch(protLevel)
                            {
                                case GattProtectionLevel::AuthenticationRequired:
                                    std::printf("\nProtection level: Authentication Required");
                                    break;
                                case GattProtectionLevel::EncryptionAndAuthenticationRequired:
                                    std::printf("\nProtection level: Encryption And Authentication Required");
                                    break;
                                case GattProtectionLevel::EncryptionRequired:
                                    std::printf("\nProtection level: Encryption Required");
                                    break;
                                case GattProtectionLevel::Plain:
                                    std::printf("\nProtection level: Plain");
                                    break;
                            }
                        }
                    }
                    else
                    {
                        LOG_WARN_FMT("failed to get descriptors. Failed with: \"{}\"", std::to_underlying(descriptorResults.Status()));
                    }
                }
            }
            else
            {
                LOG_WARN_FMT("failed to get characteristics. Failed with: \"{}\"", std::to_underlying(characteristicsResults.Status()));
            }
        }
    }
    else
    {
        LOG_WARN_FMT("failed to get service. Failed with: \"{}\"", std::to_underlying(servicesResult.Status()));
    }
}


void test_ecc_sign()
{
    security::CRandom rng = security::CRandom::make_rng().value();
    security::CEccKeyPair keyPair{ rng };
    security::CEccPublicKey pubKey = keyPair.public_key();
    security::CEccPrivateKey privKey = keyPair.private_key();
    
    const char* msg = "Very nice message";
    auto hash = security::CHash<security::Sha256>::make_hash(msg).value();
    std::vector<byte> signature = privKey.sign_hash(rng, hash);
    bool verified = pubKey.verify_hash(signature, hash);
    if (verified) {
        std::printf("Signature Verified Successfully.\n");
    } else {
        std::printf("Failed to verify Signature.\n");
    }
}


int main(int argc, char** argv)
{
    auto result = security::CWolfCrypt::instance();
    test_ecc_sign();
    return 0;
    
    
    ASSERT_FMT(0 < argc, "ARGC is {} ?!", argc);
    
    //CThreadSafeHashMap<std::string, ble::DeviceInfo> cache{};
    //
    //ble::CBLEScanner indScanner = ble::make_scanner(cache);
    //
    ////ble::win::CScanner scanner{ cache };
    ////scanner.begin_scan();
    //
    ////begin_scan(indScanner);
    //indScanner.begin_scan();
    //
    //while(true)
    //{
    //    if(cache.size() > 0)
    //    {
    //        std::vector<ble::DeviceInfo> infos = cache.as_vector();
    //        for(const auto& info : infos)
    //        {
    //            LOG_INFO_FMT("Device in cache.\nAddress: {}\nAddress Type: {}",
    //                         ble::hex_addr_to_str(info.address.value()),
    //                         ble::address_type_to_str(info.addressType));
    //
    //            query_device(info.address.value());
    //        }
    //        break;
    //    }
    //
    //    std::this_thread::sleep_for(std::chrono::seconds(1));
    //}
    
    
    // winrt::uninit_apartment();

    

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