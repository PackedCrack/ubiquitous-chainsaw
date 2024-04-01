//
// Created by qwerty on 2024-03-31.
//

#pragma once
#include "../../gfx/CWindow.hpp"
// win32
#include <shellapi.h>


namespace sys
{
class CTrayIcon
{
public:
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
    CTrayIcon(const CTrayIcon& other) = delete;
    CTrayIcon(CTrayIcon&& other) = default;
    CTrayIcon& operator=(const CTrayIcon& other) = delete;
    CTrayIcon& operator=(CTrayIcon&& other) = default;
private:
    explicit CTrayIcon(gfx::CWindow& window);
public:
    void send_balloon_info(std::string_view title, std::string_view msg, BalloonIcon iconType);
private:
    [[nodiscard]] HWND win32_window_handle() const;
private:
    std::unique_ptr<GUID> m_pGuid;
    std::reference_wrapper<gfx::CWindow> m_Window;
};
}   // namespace sys