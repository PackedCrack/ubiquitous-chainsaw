//
// Created by qwerty on 2024-03-31.
//

#pragma once
//#include "../../gfx/CWindow.hpp"
// win32
#include "SDL3/SDL.h"
#include "win32.hpp"
#include <shellapi.h>
//
//
//
//
namespace gfx
{
class CWindow;
}    // namespace gfx
namespace sys
{
class CTrayIcon
{
public:
    using MessageCallback = std::function<SDL_bool(void*, MSG*)>;
    enum class Error
    {
        constructionError
    };
    enum class BalloonIcon : DWORD
    {
        none = NIIF_NONE,
        info = NIIF_INFO,
        warning = NIIF_WARNING,
        error = NIIF_ERROR,
        user = NIIF_USER
    };
public:
    [[nodiscard]] static std::expected<CTrayIcon, Error> make(gfx::CWindow& window);
    ~CTrayIcon();
    CTrayIcon(const CTrayIcon& other, bool* pShowWindow) = delete;
    CTrayIcon(CTrayIcon&& other) noexcept;
    CTrayIcon& operator=(const CTrayIcon& other) = delete;
    CTrayIcon& operator=(CTrayIcon&& other) noexcept;
private:
    explicit CTrayIcon(gfx::CWindow& window);
public:
    [[nodiscard]] static SDL_bool SDLCALL message_hook_caller(void* pMessageCallback, MSG* pMsg);
    void send_balloon_info(std::string_view title, std::string_view msg, BalloonIcon iconType);
private:
    [[nodiscard]] sys::CTrayIcon::MessageCallback make_message_callback();
    [[nodiscard]] HWND win32_window_handle() const;
private:
    std::unique_ptr<GUID> m_pGuid;
    gfx::CWindow* m_pWindow;
    MessageCallback m_MessageCallback;
};
}    // namespace sys
