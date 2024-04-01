#pragma once
#include "defines.hpp"
// third_party
#include "../win32.hpp"
#include "SDL3/SDL.h"


namespace gfx
{
class CWindow
{
public:
	CWindow(const std::string& title, int32_t width, int32_t height, uint32_t flags);
	~CWindow();
	CWindow(const CWindow& other) = delete;
	CWindow(CWindow&& other) noexcept;
	CWindow& operator=(const CWindow& other) = delete;
	CWindow& operator=(CWindow&& other) noexcept;

	void process_events(bool* pExit) const;
	[[nodiscard]] uint32_t id() const;
	[[nodiscard]] SDL_Window* handle();
private:
	SDL_Window* m_pWindow = nullptr;
};

#ifdef WIN32
[[nodiscard]] inline std::expected<HWND, std::string_view> get_system_window_handle(CWindow& window)
{
    uint32_t propID = SDL_GetWindowProperties(window.handle());
    HWND hWindow = static_cast<HWND>(SDL_GetProperty(propID, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
    if(hWindow != nullptr)
        return std::expected<HWND, std::string_view>{ hWindow };
    else
        return std::unexpected(std::format("Failed to retrieve handle to system window from SDL. Error: \"{}\"", SDL_GetError()));
}
#else
    #error Only windows implemented
#endif
}	// namespace gfx