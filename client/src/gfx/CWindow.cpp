#include "CWindow.hpp"
#include "SDL_Defines.hpp"
#include "../resource.hpp"
// third_party
#include "imgui/imgui_impl_sdl3.h"
#include "../system/CTrayIcon.hpp"


namespace
{
}
namespace gfx
{
CWindow::CWindow(const std::string& title, int32_t width, int32_t height, uint32_t flags)
    : m_pWindow{ nullptr }
    , m_SystemTray{}
{
    SDL_CHECK(SDL_Init(SDL_INIT_VIDEO), "initalization failed");
    m_pWindow = SDL_CreateWindow(title.c_str(), width, height, flags);
    ASSERT(m_pWindow != nullptr, "Window failed to create!");
    
    {
        std::expected<HWND, std::string_view> expected = gfx::get_system_window_handle(*this);
        if(!expected)
        {
            ASSERT_FMT(m_pWindow != nullptr, "Failed to obtain Win32 window handle from SDL: {}", expected.error());
        }
    }
    
    {
        // TODO: put this on its own thread because apparently shell command can time out..
        for(int i = 0; i < 10; ++i)
        {
            std::expected<sys::CTrayIcon, sys::CTrayIcon::Error> expected = sys::CTrayIcon::make(*this);
            if(!expected)
                continue;
            
            ASSERT(expected.has_value(), "Failed to construct System Tray Icon");
            
            m_SystemTray.emplace(std::move(*expected));
            break;
        }
    }
}
CWindow::~CWindow()
{
    m_SystemTray = std::nullopt;
    
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
void CWindow::show()
{
    SDL_ShowWindow(m_pWindow);
}
void CWindow::hide()
{
    SDL_HideWindow(m_pWindow);
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
void CWindow::popup_warning(std::string_view title, std::string_view msg)
{
    using BalloonIcon = sys::CTrayIcon::BalloonIcon;
    if(m_SystemTray)
    {
        m_SystemTray->send_balloon_info(title, msg, BalloonIcon::warning);
    }
    else
    {
        // TODO: If there is no system tray then shell call likely timed out or similar
        // So retry creating it..
        LOG_ERROR("There's no system tray..");
    }
}
uint32_t CWindow::id() const
{
    ASSERT(m_pWindow, "Window handle is null!");

    uint32_t windowID = SDL_GetWindowID(m_pWindow);
    ASSERT(windowID, "Failed to retrieve window id!");

    return windowID;
}
SDL_Window* CWindow::handle()
{
    ASSERT(m_pWindow, "Window handle is null!");

    return m_pWindow;
}
}	// namespace gfx