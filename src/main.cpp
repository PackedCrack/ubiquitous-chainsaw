#include "defines.hpp"
#include "gfx/CWindow.hpp"
#include "gfx/CRenderer.hpp"
#include "gfx/SDL_Defines.hpp"
#include "gui/CGui.hpp"

#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>


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

#include <iostream>
#include <string>
#include "bluetoothLE/windows/CScanner.hpp"

int main(int argc, char** argv)
{
    ASSERT_FMT(0 < argc, "ARGC is {} ?!", argc);
    
    CThreadSafeHashMap<std::string, ble::DeviceInfo> cache{};
    ble::win::CScanner scanner{ cache };
    
    scanner.begin_scan();
    
    while(true)
    {
        if(cache.size() > 0)
        {
            std::vector<ble::DeviceInfo> infos = cache.as_vector();
            for(const auto& info : infos)
            {
                LOG_INFO_FMT("Device in cache.\nAddress: {}\nAddress Type: {}", info.address.value(), std::to_underlying(info.addressType));
            }
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