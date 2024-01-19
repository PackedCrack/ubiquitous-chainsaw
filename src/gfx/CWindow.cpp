#include "CWindow.hpp"
#include "SDL_Defines.hpp"
// third_party
#include "SDL3/SDL.h"
#include "SDL3/SDL_events.h"
#include "imgui/imgui_impl_sdl3.h"


namespace gfx
{
CWindow::CWindow(const std::string& title, int32_t width, int32_t height, uint32_t flags)
    : m_pWindow{ nullptr }
{
    SDL_CHECK(SDL_Init(SDL_INIT_VIDEO), "initalization failed");
    m_pWindow = SDL_CreateWindow(title.c_str(), width, height, flags);
    ASSERT(m_pWindow != nullptr, "Window failed to create!");
}
CWindow::~CWindow()
{
    if (!m_pWindow)
        return;

    SDL_DestroyWindow(m_pWindow);
}

CWindow::CWindow(CWindow&& other) noexcept
{
    std::swap(m_pWindow, other.m_pWindow);
}

CWindow& CWindow::operator=(CWindow&& other) noexcept
{
    std::swap(m_pWindow, other.m_pWindow);

    return *this;
}

void CWindow::process_events(bool* pExit) const
{
    SDL_Event evnt{};
    while (SDL_PollEvent(&evnt))
    {
        ImGui_ImplSDL3_ProcessEvent(&evnt);
        if (evnt.type == SDL_EVENT_QUIT)
        {
            *pExit = true;
        }
        if (evnt.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        {
            *pExit = true;
        }
    }
}

uint32_t CWindow::id() const
{
    ASSERT(m_pWindow, "Window handle is null!");

    uint32_t windowID = SDL_GetWindowID(m_pWindow);
    ASSERT(windowID, "Failed to retrieve window id!");

    return windowID;
}

SDL_Window* CWindow::handle() const
{
    ASSERT(m_pWindow, "Window handle is null!");

    return m_pWindow;
}
}	// namespace gfx