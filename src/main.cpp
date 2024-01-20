#include "defines.hpp"
#include "gfx/CWindow.hpp"
#include "gfx/CRenderer.hpp"
#include "gfx/SDL_Defines.hpp"
#include "gui/CGui.hpp"
// third_party
#include "SDL3/SDL.h"


#include "bluetooth/CScanner.hpp"

namespace
{
void process_cmd_line_args(int argc, char** argv)
{
    if (argc < 1)
        LOG_FATAL_FMT("Not enough command line arguments given, expected at least 1.", argc);

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

[[nodiscard]] std::span<const bluetooth::Inquiry> to_span(
        const std::vector<bluetooth::Inquiry>& inquiries)
{
    return inquiries;
}
[[nodiscard]] std::expected<std::span<const bluetooth::Inquiry>, bluetooth::CScanner::Error> print_inquiries(
        std::span<const bluetooth::Inquiry> inquiries)
{
    for(const auto& inquiry : inquiries)
    {
        std::string services{};
        for(size_t i = 0u; i < inquiry.services.size(); ++i)
        {
            bluetooth::ServiceClass service = inquiry.services[i];
            services += bluetooth::service_class_to_str(service);
            if (i < inquiry.services.size() - 1u)
                services += ", ";
        }
        
        LOG_INFO_FMT("Bluetooth Device:\nName: {}\nAddress: {}\nService Class: {}\nMajor Device Class: {}",
                     inquiry.name,
                     inquiry.addr,
                     services,
                     bluetooth::major_device_class_to_str(inquiry.majorClass));
    }
    
    return inquiries;
}
[[nodiscard]] std::expected<std::span<const bluetooth::Inquiry>, bluetooth::CScanner::Error> log_scan_err(
        const bluetooth::CScanner::Error& err)
{
    LOG_ERROR_FMT("Scanned failed with Error Code: {} and Message: {}",
                  std::to_underlying(err.code),
                  err.msg);
    
    return std::unexpected{ err };
}
}   // namespace


int main(int argc, char** argv)
{
    ASSERT_FMT(0 < argc, "ARGC is {} ?!", argc);

    try
    {
        process_cmd_line_args(argc, argv);
        
        
        std::expected<bluetooth::CScanner, bluetooth::CScanner::Error> result = bluetooth::CScanner::make_scanner();
        if(!result)
        {
            const bluetooth::CScanner::Error& err = result.error();
            LOG_FATAL_FMT("Failed to create Bluetooth scanner.\nError Code: {}\nMessage: {}",
                          std::to_underlying(err.code),
                          err.msg);
        }
        
        bluetooth::CScanner scanner = std::move(result.value());
        scanner.new_scan()
        .transform(to_span)
        .and_then(print_inquiries)
        .or_else(log_scan_err);
        
        
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


        return 0;
    }
    catch(const exception::fatal_error& err)
    {
        LOG_ERROR_FMT("Fatal exception: {}", err.what());
        return -1;
    }
}