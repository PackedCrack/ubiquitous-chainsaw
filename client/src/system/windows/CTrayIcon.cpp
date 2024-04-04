//
// Created by qwerty on 2024-03-31.
//
#include "CTrayIcon.hpp"
#include "../../client_defines.hpp"
#include "../../resource.hpp"
#include "../../gfx/CWindow.hpp"


namespace
{
[[nodiscard]] std::unique_ptr<GUID> make_guid()
{
    // powershell: [guid]::NewGuid()
    // bc 5d-c6 f9 6b f8 aa 5e
    static constexpr std::array<GUID, 1> guids{
            { 0xDA5FB24F, 0x5CF9, 0x4C55, { 0xCB, 0x5D, 0xC6, 0xF9, 0x6B, 0xF8, 0xAA, 0x5E }}
    };
    static size_t inUse = 0u;
    ASSERT(inUse < guids.size(), "TODO: an actual guid generating function..");
    
    return std::make_unique<GUID>(guids[inUse]);
}
[[nodiscard]] std::optional<HMODULE> get_module_handle()
{
    // https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulehandlea
    HMODULE moduleHandle = nullptr;
    WIN_CHECK(moduleHandle = GetModuleHandleA(nullptr); moduleHandle != nullptr);
    
    return moduleHandle != nullptr ? std::optional<HMODULE>{ moduleHandle } : std::nullopt;
}
[[nodiscard]] std::optional<HANDLE> load_image(HMODULE hModule)
{
    static constexpr int32_t USE_DEFAULT_XY = 0;
    static constexpr uint32_t LOAD_FLAGS = LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_LOADTRANSPARENT | LR_SHARED;
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadimagea
    HANDLE handle = nullptr;
    WIN_CHECK(handle = LoadImage(
            hModule,
            MAKEINTRESOURCE(IDI_APPICON),
            IMAGE_ICON,
            USE_DEFAULT_XY,
            USE_DEFAULT_XY,
            LOAD_FLAGS); handle != nullptr);
    
    return handle != nullptr ? std::optional<HANDLE>{ handle } : std::nullopt;
}
[[nodiscard]] auto create_data(HWND hWindow, const GUID& guid)
{
    return [hWindow, &guid](HANDLE iconHandle) -> std::optional<NOTIFYICONDATAA>
    {
        // https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-notifyicondataa
        return NOTIFYICONDATAA{
                .cbSize = sizeof(NOTIFYICONDATA),
                .hWnd = hWindow,
                .uID = APPICON_UID,
                .uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP, //| NIF_GUID,    // NIF_INFO | NIF_REALTIME
                .uCallbackMessage = SYSTRAY_ICON_EVNT,
                .hIcon = static_cast<HICON>(iconHandle),
                .szTip = APPNAME,
                .dwState = NIS_SHAREDICON,
                .dwStateMask = 0,
                .szInfo = { 0 },
                //.uTimeout = 0, // This member is deprecated as of Windows Vista. Notification display times are now based on system accessibility settings
                //.uVersion = 0 Union with uTimeout (deprecated as of Windows Vista).
                //.uVersion = NOTIFYICON_VERSION_4,
                .uVersion = NOTIFYICON_VERSION_4,
                .szInfoTitle = { 0 },
                .dwInfoFlags = NIIF_WARNING | NIIF_USER, // https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ne-shellapi-shstockiconid
                .guidItem = guid,
                .hBalloonIcon = nullptr
        };
    };
}
[[nodiscard]] std::optional<NOTIFYICONDATAA> log_failure()
{
    LOG_ERROR("Failed to create Win32 Notify Icon Data");
    return std::nullopt;
}
[[nodiscard]] std::optional<NOTIFYICONDATAA> make_notify_con_data(HWND hWindow, const GUID& guid)
{
    std::optional<NOTIFYICONDATAA> data = get_module_handle()
    .and_then(load_image)
    .and_then(create_data(hWindow, guid))
    .or_else(log_failure);
    
    return data;
}
[[nodiscard]] std::optional<NOTIFYICONDATAA> make_notify_con_data_balloon(
        HWND hWindow,
        const GUID& guid,
        std::string_view title,
        std::string_view msg,
        sys::CTrayIcon::BalloonIcon iconType)
{
    std::optional<NOTIFYICONDATAA> data = make_notify_con_data(hWindow, guid);
    if(data)
    {
        data->uFlags = data->uFlags | NIF_INFO;
        
        std::memset(data->szInfoTitle, '\0', sizeof(data->szInfoTitle));
        std::memcpy(data->szInfoTitle, title.data(), title.size() < sizeof(data->szInfoTitle) ? title.size() : sizeof(data->szInfoTitle));
        
        std::memset(data->szInfo, '\0', sizeof(data->szInfo));
        std::memcpy(data->szInfo, msg.data(), msg.size() < sizeof(data->szInfo) ? msg.size() : sizeof(data->szInfo));
        
        data->dwInfoFlags = std::to_underlying(iconType);
    }
    
    /*
     * The handle of a customized notification icon provided by the application that should be used independently
     * of the notification area icon. If this member is non-NULL and the NIIF_USER (CTrayIcon::BalloonIcon::user) flag is set in the dwInfoFlags member,
     * this icon is used as the notification icon. If this member is NULL, the legacy behavior is carried out.
    */
    // data.hBalloonIcon = static_cast<HICON>(balloonIcon);
    
    
    return data;
}
}   // namespace
namespace sys
{
SDL_bool SDLCALL CTrayIcon::message_hook_caller(void* pMessageCallback, MSG* pMsg)
{
    MessageCallback& cb = *static_cast<MessageCallback*>(pMessageCallback);
    return cb(nullptr, pMsg);
}
std::expected<CTrayIcon, CTrayIcon::Error> CTrayIcon::make(gfx::CWindow& window)
{
    try
    {
        // This move is not cheap ... TODO:: fix api
        return std::expected<CTrayIcon, CTrayIcon::Error>{ CTrayIcon{ window } };
    }
    catch(const std::runtime_error& err)
    {
        LOG_ERROR_FMT("{}", err.what());
        return std::unexpected(Error::constructionError);
    }
}
CTrayIcon::CTrayIcon(gfx::CWindow& window)
    : m_pGuid{ make_guid() }
    , m_pWindow{ &window }
    , m_MessageCallback{ make_message_callback() }
{
    HWND hWindow = win32_window_handle();
    if(hWindow != nullptr)
    {
        // make_tray_icon
        std::optional<NOTIFYICONDATAA> data = make_notify_con_data(hWindow, *m_pGuid);
        if(data)
        {
            NOTIFYICONDATAA& d = *data;
            BOOL result = Shell_NotifyIcon(NIM_ADD, &d);
            if(result == FALSE)
                throw std::runtime_error{ "Failed to create system try icon. Requested version is not supported" };
            
            SDL_SetWindowsMessageHook(message_hook_caller, &m_MessageCallback);
        }
        else
        {
            throw std::runtime_error{ "CTrayIcon constructor cannot continue without Notify Icon Data." };
        }
    }
    else
    {
        throw std::runtime_error{ "CTrayIcon constructor cannot continue without a handle to win32 window" };
    }
}
CTrayIcon::~CTrayIcon()
{
    if(m_pGuid)
    {
        HWND wHandle = win32_window_handle();
        
        if(wHandle != nullptr)
        {
            SDL_SetWindowsMessageHook(nullptr, nullptr);
            
            std::optional<NOTIFYICONDATAA> data = make_notify_con_data(wHandle, *m_pGuid);
            if(data)
            {
                NOTIFYICONDATAA& d = *data;
                BOOL result = Shell_NotifyIcon(NIM_DELETE, &d);
                if(result == false)
                {
                    LOG_ERROR("Shell_NotifyIcon failed to create Balloon notification.");
                }
            }
            else
            {
                LOG_ERROR("Could not create the necessary Notify Icon Data");
            }
        }
    }
}
CTrayIcon::CTrayIcon(CTrayIcon&& other) noexcept
    : m_pGuid{ std::move(other.m_pGuid) }
    , m_pWindow{ std::exchange(other.m_pWindow, nullptr) }
    , m_MessageCallback{ std::move(other.m_MessageCallback) }
{
    SDL_SetWindowsMessageHook(message_hook_caller, &m_MessageCallback);
}
CTrayIcon& CTrayIcon::operator=(CTrayIcon&& other) noexcept
{
    m_pGuid = std::move(other.m_pGuid);
    m_pWindow = std::exchange(other.m_pWindow, nullptr);
    m_MessageCallback = std::move(other.m_MessageCallback);
    
    SDL_SetWindowsMessageHook(message_hook_caller, &m_MessageCallback);
    
    return *this;
}
void CTrayIcon::send_balloon_info(std::string_view title, std::string_view msg, BalloonIcon iconType)
{
    ASSERT(!title.empty(), "Title must be provided!");
    ASSERT(!msg.empty(), "Title must be provided!");
    
    HWND wHandle = win32_window_handle();
    if(wHandle != nullptr)
    {
        std::optional<NOTIFYICONDATAA> data = make_notify_con_data_balloon(wHandle, *m_pGuid, title, msg, iconType);
        if(data)
        {
            NOTIFYICONDATAA& d = *data;
            BOOL result = Shell_NotifyIcon(NIM_MODIFY, &d);
            if(result == false)
            {
                LOG_ERROR("Shell_NotifyIcon failed to create Balloon notification.");
            }
        }
        else
        {
            LOG_ERROR("Could not create the necessary Notify Icon Data");
        }
    }
}
HWND CTrayIcon::win32_window_handle() const
{
    std::expected<HWND, std::string_view> expected = gfx::get_system_window_handle(*m_pWindow);
    if(!expected)
    {
        LOG_ERROR_FMT("Failed to obtain Win32 window handle from SDL: {}", expected.error());
        return nullptr;
    }
    
    return *expected;
}
sys::CTrayIcon::MessageCallback CTrayIcon::make_message_callback()
{
    // TODO: SDL does not forward this Notify Icon Tray message correctly.
    return [pWindow = m_pWindow]([[maybe_unused]] void* pMessageCallback, MSG* pMsg)
    {
        switch (pMsg->message)
        {
            case SYSTRAY_ICON_EVNT:
            {
                LOG_INFO("Tray Event");
                break;
            }
            //default:
            //    LOG_INFO_FMT("Unknown message: \"{}\"", pMsg->message);
        }
        
        return SDL_TRUE;
    };
}
}   // namespace sys