#include "defines.hpp"
#include "gfx/CWindow.hpp"
#include "gfx/CRenderer.hpp"
#include "gfx/SDL_Defines.hpp"
#include "gui/CGui.hpp"
// third_party
#include "SDL3/SDL.h"


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
}   // namespace


int main(int argc, char** argv)
{
    ASSERT_FMT(0 < argc, "ARGC is {} ?!", argc);

    try
    {
        process_cmd_line_args(argc, argv);
        
        gui::CGui gui{ };

        gfx::CWindow window{ "Some title", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY };
        gfx::CRenderer renderer{ window, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED };


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