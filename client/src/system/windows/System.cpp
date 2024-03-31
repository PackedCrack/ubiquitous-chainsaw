//
// Created by qwerty on 2024-03-01.
//

#include "System.h"
#include "defines.hpp"
// win32
//#include <winuser.h>
#include <windows.h>
//#include <shellapi.h>
// winrt
#include <winrt/windows.foundation.h>


namespace
{
[[nodiscard]] NOTIFYICONDATAA make_notify_con_data(HWND hWindow, HANDLE iconHandle)
{
    // powershell: [guid]::NewGuid()
    // bc 5d-c6 f9 6b f8 aa 5e
    static constexpr GUID guid =
            { 0xDA5FB24F, 0x5CF9, 0x4C55, { 0xCB, 0x5D, 0xC6, 0xF9, 0x6B, 0xF8, 0xAA, 0x5E }};
    
    // https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-notifyicondataa
    return NOTIFYICONDATAA{
        .cbSize = sizeof(NOTIFYICONDATA),
        .hWnd = hWindow,
        .uID = 0,
        .uFlags = NIF_ICON | NIF_TIP | NIF_GUID,    // NIF_INFO | NIF_REALTIME
        .uCallbackMessage = 0,
        .hIcon = static_cast<HICON>(iconHandle),
        .szTip = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
        .dwState = NIS_SHAREDICON,
        .dwStateMask = 0,
        .szInfo = "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII",
        .uTimeout = 0, // This member is deprecated as of Windows Vista. Notification display times are now based on system accessibility settings
        //.uVersion = 0 Union with uTimeout (deprecated as of Windows Vista).
        .szInfoTitle = "OOOOOOOOOOOOOOOOOOOOOOO",
        .dwInfoFlags = NIIF_WARNING | NIIF_USER, // https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ne-shellapi-shstockiconid
        .guidItem = guid,
        .hBalloonIcon = static_cast<HICON>(iconHandle)
    };
}
[[nodiscard]] std::optional<HMODULE> get_module_handle()
{
    // https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulehandlea
    HMODULE moduleHandle = nullptr;
    WIN_CHECK(moduleHandle = GetModuleHandleA(nullptr); moduleHandle != nullptr);
    
    return std::optional<HMODULE>{ moduleHandle };
}
[[nodiscard]] std::optional<HANDLE> load_image(HMODULE hModule)
{
    static constexpr int32_t USE_DEFAULT_XY = 0;
    static constexpr uint32_t LOAD_FLAGS = LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_LOADTRANSPARENT | LR_SHARED;
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadimagea
    HANDLE handle = nullptr;
    WIN_CHECK(handle = LoadImage(hModule, "icon.ico", IMAGE_ICON, USE_DEFAULT_XY, USE_DEFAULT_XY, LOAD_FLAGS); handle != nullptr);
    
    return std::optional<HANDLE>{ handle };
}
}   // namespace
namespace sys
{
System::System()
{
    winrt::init_apartment();
}
System::~System()
{
    winrt::uninit_apartment();
}
bool System::make_system_tray(HWND hWindow) const
{
    std::optional<HMODULE> hModule = get_module_handle();
    if(!hModule)
    {
        LOG_ERROR("Failed to retrieve handle to module when trying to create the system tray.");
        return false;
    }
    std::optional<HANDLE> hIcon = load_image(*hModule);
    if(!hIcon)
    {
        LOG_ERROR("Failed to retrieve handle to icon when trying to create the system tray.");
        return false;
    }
    
    // https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shell_notifyicona
    static auto data = make_notify_con_data(hWindow, *hIcon);
    // https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shell_notifyicona
    BOOL result = Shell_NotifyIcon(NIM_ADD, &data);
    if(result == false)
    {
        LOG_ERROR_FMT("Failed to create system try icon. Requested version is not supported");
    }
    
    return result;
}
bool System::free_system_tray(HWND hWindow) const
{
    std::optional<HMODULE> hModule = get_module_handle();
    if(!hModule)
    {
        LOG_ERROR("Failed to retrieve handle to module when trying to create the system tray.");
        return false;
    }
    std::optional<HANDLE> hIcon = load_image(*hModule);
    if(!hIcon)
    {
        LOG_ERROR("Failed to retrieve handle to icon when trying to create the system tray.");
        return false;
    }
    
    // https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shell_notifyicona'
    static auto data = make_notify_con_data(hWindow, *hIcon);
    BOOL result = Shell_NotifyIcon(NIM_DELETE, &data);
    if(result == false)
    {
        LOG_ERROR_FMT("Failed to create system try icon. Requested version is not supported");
    }
    
    return result;
}
}   // namespace sys
