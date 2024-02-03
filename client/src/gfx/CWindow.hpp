#pragma once
#include "../defines.hpp"
// third_party
#include "SDL3/SDL_video.h"


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
	[[nodiscard]] SDL_Window* handle() const;
private:
	SDL_Window* m_pWindow = nullptr;
};
}	// namespace gfx